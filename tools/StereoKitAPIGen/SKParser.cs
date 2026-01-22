using System.Text.RegularExpressions;

namespace StereoKitAPIGen;

/// <summary>
/// Custom regex-based parser for StereoKit C header files.
/// Parses enums, structs, functions, constants, and opaque types.
/// </summary>
public static class SKParser {
	// ─────────────────────────────────────────────────────────────
	// Regex Patterns
	// ─────────────────────────────────────────────────────────────

	// Enum start: captures (1) comment, (2) name
	// Note: We use manual brace matching for the body to handle } in comments
	static readonly Regex EnumStartRegex = new(
		@"(/\*[^*]*(?:\*(?!/)[^*]*)*\*/)?\s*typedef\s+enum\s+(\w+)\s*\{",
		RegexOptions.Singleline | RegexOptions.Compiled);

	// Enum value: captures (1) comment, (2) name, (3) value expression
	static readonly Regex EnumValueRegex = new(
		@"(?:/\*([^*]*(?:\*(?!/)[^*]*)*)\*/)?\s*(\w+)\s*(?:=\s*([^,/]+))?\s*,?",
		RegexOptions.Singleline | RegexOptions.Compiled);

	// Struct/Union: captures (1) struct|union, (2) name, (3) body
	static readonly Regex StructRegex = new(
		@"(/\*[^*]*(?:\*(?!/)[^*]*)*\*/)?\s*typedef\s+(struct|union)\s+(\w+)\s*\{([^}]+)\}\s*\3\s*;",
		RegexOptions.Singleline | RegexOptions.Compiled);

	// Struct field: captures (1) comment, (2) type, (3) name, (4) array size
	static readonly Regex FieldRegex = new(
		@"(?:/\*([^*]*(?:\*(?!/)[^*]*)*)\*/)?\s*([\w\s*]+?)\s+(\w+)(?:\[(\d+)\])?\s*;",
		RegexOptions.Singleline | RegexOptions.Compiled);

	// Function: captures (1) deprecated marker, (2) return type, (3) name
	// Note: We use a custom matcher instead of regex for functions due to nested parentheses in sk_default()
	// The pattern handles pointer * attached to either type or name: "char* name" or "char *name"
	// Use lazy +? to avoid greedy matching of function name into return type
	static readonly Regex FunctionStartRegex = new(
		@"SK_API\s+(SK_DEPRECATED\s+)?([\w\s*]+?)\s+\*?(\w+)\s*\(",
		RegexOptions.Singleline | RegexOptions.Compiled);

	// Function pointer in parameter: captures (1) return type, (2) name
	// Note: We extract params manually to handle nested parens from sk_ref()
	static readonly Regex FunctionPtrStartRegex = new(
		@"([\w\s*]+)\s*\(\s*\*\s*(\w+)\s*\)\s*\(",
		RegexOptions.Compiled);

	// Opaque type: captures (1) name
	static readonly Regex OpaqueTypeRegex = new(
		@"SK_DeclarePrivateType\s*\(\s*(\w+)\s*\)\s*;",
		RegexOptions.Compiled);

	// Constant: captures (1) type, (2) name, (3) value
	static readonly Regex ConstantRegex = new(
		@"SK_CONST\s+([\w\s*]+?)\s+(\w+)\s*=\s*([^;]+);",
		RegexOptions.Compiled);

	// Bitflag marker: captures (1) enum name
	static readonly Regex BitflagRegex = new(
		@"SK_MakeFlag\s*\(\s*(\w+)\s*\)\s*;?",
		RegexOptions.Compiled);

	// Default parameter: captures (1) value
	static readonly Regex DefaultParamRegex = new(
		@"\s*sk_default\s*\(\s*([^)]*)\s*\)",
		RegexOptions.Compiled);

	// sk_ref() macro: captures (1) type
	static readonly Regex RefMacroRegex = new(
		@"(?:const\s+)?sk_ref(?:_arr)?\s*\(\s*(\w+)\s*\)",
		RegexOptions.Compiled);

	// Bit shift expression: captures (1) base, (2) shift amount
	// Internal so BindZig can also use it
	internal static readonly Regex BitShiftRegex = new(
		@"(\d+)\s*<<\s*(\d+)",
		RegexOptions.Compiled);

	// Array notation in name: captures (1) name, (2) size
	static readonly Regex NameArrayRegex = new(
		@"^(\w+)\[(\d+)\]$",
		RegexOptions.Compiled);

	// Array notation in type: captures (1) type, (2) size1, (3) size2 optional
	static readonly Regex TypeArrayRegex = new(
		@"^(.+?)\s*\[(\d+)\](?:\s*\[(\d+)\])?$",
		RegexOptions.Compiled);

	// Whitespace collapse pattern
	static readonly Regex WhitespaceRegex = new(
		@"[ \t]+",
		RegexOptions.Compiled);

	// ─────────────────────────────────────────────────────────────
	// Module Override Pattern (for SKModules.txt)
	// ─────────────────────────────────────────────────────────────

	public class ModuleOverride {
		public string Name   { get; set; } = "";
		public string Prefix { get; set; } = "";
	}

	// ─────────────────────────────────────────────────────────────
	// Main Parse Method
	// ─────────────────────────────────────────────────────────────

	public static SKHeaderData Parse(string[] filePaths, List<ModuleOverride>? moduleOverrides = null) {
		var data = new SKHeaderData();
		var bitflagEnums = new HashSet<string>();

		foreach (var filePath in filePaths) {
			string content = File.ReadAllText(filePath);
			ParseFile(content, data);
			CollectBitflagMarkers(content, bitflagEnums);
		}

		// Detect bitflags
		DetectBitflags(data, bitflagEnums);

		// Build modules from structs, unions, and opaque types
		BuildModules(data, moduleOverrides ?? new List<ModuleOverride>());

		// Sort modules once for all association methods (longest prefix first for best matching)
		var sortedModules = data.Modules.OrderByDescending(m => m.Prefix.Length).ToList();

		// Associate items with their modules
		AssociateFunctionsWithModules(data, sortedModules);
		AssociateEnumsWithModules(data, sortedModules);
		AssociateConstantsWithModules(data, sortedModules);

		return data;
	}

	// ─────────────────────────────────────────────────────────────
	// File Parsing
	// ─────────────────────────────────────────────────────────────

	static void ParseFile(string content, SKHeaderData data) {
		// First pass: collect opaque types so we can identify asset handles
		foreach (Match match in OpaqueTypeRegex.Matches(content)) {
			data.OpaqueTypes.Add(match.Groups[1].Value);
		}

		// Parse enums (using manual brace matching for body)
		ParseEnums(content, data);

		// Parse structs and unions
		foreach (Match match in StructRegex.Matches(content)) {
			var module = ParseStructOrUnion(match);
			if (module != null) data.Modules.Add(module);
		}

		// Parse functions (using custom parser for nested parentheses in sk_default)
		ParseFunctions(content, data);

		// Parse constants
		foreach (Match match in ConstantRegex.Matches(content)) {
			var constant = ParseConstant(match);
			if (constant != null) data.Constants.Add(constant);
		}
	}

	// ─────────────────────────────────────────────────────────────
	// Function Parsing (with nested parenthesis handling)
	// ─────────────────────────────────────────────────────────────

	static void ParseFunctions(string content, SKHeaderData data) {
		// Find all function starts with regex, then manually extract parameters
		// handling nested parentheses from sk_default()
		foreach (Match match in FunctionStartRegex.Matches(content)) {
			int parenStart = match.Index + match.Length - 1; // Position of opening '('
			int parenEnd = FindMatchingClose(content, parenStart, '(', ')');
			if (parenEnd < 0) continue; // Malformed, skip

			// Extract parameter string (between the parens)
			string paramsStr = content[(parenStart + 1)..parenEnd].Trim();

			// Check for semicolon after closing paren
			int semicolonPos = content.IndexOf(';', parenEnd);
			if (semicolonPos < 0 || semicolonPos > parenEnd + 10) continue; // Must be close after )

			bool   isDeprecated = match.Groups[1].Success;
			string returnType   = match.Groups[2].Value.Trim();
			string name         = match.Groups[3].Value.Trim();

			// Check if there's a * between return type and name (e.g., "const char *name")
			int typeEnd = match.Groups[2].Index + match.Groups[2].Length;
			int nameStart = match.Groups[3].Index;
			string between = content[typeEnd..nameStart];
			if (between.Contains('*')) {
				returnType += "*";
			}

			var func = new SKFunction {
				Name         = name,
				ReturnType   = ParseType(returnType),
				IsDeprecated = isDeprecated,
			};

			// Parse parameters
			if (!string.IsNullOrWhiteSpace(paramsStr) && paramsStr != "void") {
				func.Parameters = ParseParameters(paramsStr);
			}

			data.Functions.Add(func);
		}
	}

	// ─────────────────────────────────────────────────────────────
	// Enum Parsing
	// ─────────────────────────────────────────────────────────────

	static void ParseEnums(string content, SKHeaderData data) {
		foreach (Match match in EnumStartRegex.Matches(content)) {
			int braceStart = match.Index + match.Length - 1; // Position of opening '{'
			int braceEnd = FindMatchingClose(content, braceStart, '{', '}');
			if (braceEnd < 0) continue; // Malformed, skip
			string body = content[(braceStart + 1)..braceEnd].Trim();

			// Verify closing pattern: } name_;
			string name = match.Groups[2].Value;
			int semiPos = content.IndexOf(';', braceEnd);
			if (semiPos < 0 || semiPos > braceEnd + name.Length + 5) continue;

			string closing = content[braceEnd..(semiPos + 1)].Trim();
			if (!closing.EndsWith($"{name};")) continue;

			var skEnum = ParseEnumBody(match.Groups[1].Value, name, body);
			if (skEnum != null) data.Enums.Add(skEnum);
		}
	}

	static SKEnum? ParseEnumBody(string comment, string name, string body) {
		comment = CleanComment(comment);

		var skEnum = new SKEnum {
			Name    = name,
			Comment = comment,
		};

		// Parse enum values
		foreach (Match valueMatch in EnumValueRegex.Matches(body)) {
			string valueName = valueMatch.Groups[2].Value.Trim();
			if (string.IsNullOrWhiteSpace(valueName)) continue;

			var item = new SKEnumItem {
				Name      = valueName,
				ValueExpr = valueMatch.Groups[3].Success ? valueMatch.Groups[3].Value.Trim() : null,
				Comment   = CleanComment(valueMatch.Groups[1].Value),
			};

			// Try to evaluate simple numeric expressions
			item.ValueNumeric = TryEvaluateExpr(item.ValueExpr, skEnum.Items);

			skEnum.Items.Add(item);
		}

		return skEnum.Items.Count > 0 ? skEnum : null;
	}

	static long? TryEvaluateExpr(string? expr, List<SKEnumItem> previousItems) {
		if (string.IsNullOrWhiteSpace(expr)) return null;

		expr = expr.Trim();

		// Simple integer
		if (long.TryParse(expr, out long val)) return val;

		// Hex value
		if (expr.StartsWith("0x", StringComparison.OrdinalIgnoreCase)) {
			if (long.TryParse(expr[2..], System.Globalization.NumberStyles.HexNumber, null, out val))
				return val;
		}

		// Bit shift: 1 << N
		var shiftMatch = BitShiftRegex.Match(expr);
		if (shiftMatch.Success) {
			int baseVal = int.Parse(shiftMatch.Groups[1].Value);
			int shift   = int.Parse(shiftMatch.Groups[2].Value);
			return baseVal << shift;
		}

		// Reference to previous value
		foreach (var prev in previousItems) {
			if (expr == prev.Name && prev.ValueNumeric.HasValue)
				return prev.ValueNumeric;
		}

		// Complex expressions - return null (store expression as string)
		return null;
	}

	// ─────────────────────────────────────────────────────────────
	// Struct/Union Parsing
	// ─────────────────────────────────────────────────────────────

	static SKModule? ParseStructOrUnion(Match match) {
		string comment   = CleanComment(match.Groups[1].Value);
		string kind      = match.Groups[2].Value; // "struct" or "union"
		string name      = match.Groups[3].Value;
		string body      = match.Groups[4].Value;

		var module = new SKModule {
			Name    = name,
			Prefix  = name + "_",
			Comment = comment,
			Type    = kind == "union" ? SKModuleType.Union : SKModuleType.Struct,
		};

		// Parse fields
		foreach (Match fieldMatch in FieldRegex.Matches(body)) {
			string fieldComment = CleanComment(fieldMatch.Groups[1].Value);
			string typeStr      = fieldMatch.Groups[2].Value.Trim();
			string fieldName    = fieldMatch.Groups[3].Value.Trim();
			string arraySize    = fieldMatch.Groups[4].Value;

			var field = new SKField {
				Name    = fieldName,
				Type    = ParseType(typeStr),
				Comment = fieldComment,
			};

			if (!string.IsNullOrEmpty(arraySize) && int.TryParse(arraySize, out int size)) {
				field.Type.ArraySize1 = size;
			}

			module.Fields.Add(field);
		}

		return module;
	}

	static List<SKParameter> ParseParameters(string paramsStr) {
		var parameters = new List<SKParameter>();

		// Split by comma, but be careful of function pointers
		var paramParts = SplitParameters(paramsStr);

		foreach (var paramStr in paramParts) {
			var param = ParseParameter(paramStr.Trim());
			if (param != null) parameters.Add(param);
		}

		return parameters;
	}

	static List<string> SplitParameters(string paramsStr) {
		var parts = new List<string>();
		int parenDepth = 0;
		int start = 0;

		for (int i = 0; i < paramsStr.Length; i++) {
			char c = paramsStr[i];
			if (c == '(') parenDepth++;
			else if (c == ')') parenDepth--;
			else if (c == ',' && parenDepth == 0) {
				parts.Add(paramsStr[start..i]);
				start = i + 1;
			}
		}

		if (start < paramsStr.Length) {
			parts.Add(paramsStr[start..]);
		}

		return parts;
	}

	static SKParameter? ParseParameter(string paramStr) {
		if (string.IsNullOrWhiteSpace(paramStr)) return null;

		// Extract and remove default value
		string? defaultValue = null;
		var defaultMatch = DefaultParamRegex.Match(paramStr);
		if (defaultMatch.Success) {
			defaultValue = defaultMatch.Groups[1].Value.Trim();
			paramStr = paramStr[..defaultMatch.Index].Trim();
		}

		// Check for function pointer - use custom parser for nested parens
		var funcPtrMatch = FunctionPtrStartRegex.Match(paramStr);
		if (funcPtrMatch.Success) {
			return ParseFunctionPointerParam(paramStr, funcPtrMatch, defaultValue);
		}

		// Handle sk_ref() and sk_ref_arr() macros
		bool isRefArr = paramStr.Contains("sk_ref_arr");
		bool isRef    = paramStr.Contains("sk_ref(") || isRefArr;
		bool isConstRef = isRef && paramStr.Contains("const");
		var refMatch = RefMacroRegex.Match(paramStr);
		if (refMatch.Success) {
			string refType = refMatch.Groups[1].Value;
			paramStr = RefMacroRegex.Replace(paramStr, refType + "*");
			if (isRefArr) {
				// sk_ref_arr adds another pointer level
				paramStr = paramStr.Replace(refType + "*", refType + "**");
			}
		}

		// Split type and name (name is the last word)
		paramStr = paramStr.Trim();
		int lastSpace = paramStr.LastIndexOf(' ');
		if (lastSpace < 0) {
			// Could be just a type like "void"
			return null;
		}

		// Handle pointer attached to name: "type *name" or "type* name"
		string typeStr = paramStr[..lastSpace].Trim();
		string name    = paramStr[(lastSpace + 1)..].Trim();

		// If name starts with *, it's part of the type
		while (name.StartsWith('*')) {
			typeStr += "*";
			name = name[1..];
		}

		// Handle array notation in name: foo[16]
		int? arraySize = null;
		var nameArrayMatch = NameArrayRegex.Match(name);
		if (nameArrayMatch.Success) {
			name = nameArrayMatch.Groups[1].Value;
			arraySize = int.Parse(nameArrayMatch.Groups[2].Value);
		}

		var param = new SKParameter {
			Name         = name,
			NameFlagless = ExtractCleanName(name),
			Type         = ParseType(typeStr),
			DefaultValue = defaultValue,
			IsRefArr     = isRefArr, // Track sk_ref_arr for double-pointer array returns
		};

		// Apply array size from name
		if (arraySize.HasValue) {
			param.Type.ArraySize1 = arraySize.Value;
			param.IsArray = true;
		}

		// Set PassType from sk_ref() macro (before ExtractParamSemantics which may override)
		if (isRef && !isRefArr) {
			param.PassType = isConstRef ? SKPassType.In : SKPassType.Ref;
		}

		// Extract semantic info from parameter name (can override PassType based on in_/out_/ref_ prefix)
		ExtractParamSemantics(param);

		return param;
	}

	static SKParameter? ParseFunctionPointerParam(string fullParam, Match match, string? defaultValue) {
		string returnTypeStr = match.Groups[1].Value.Trim();
		string name          = match.Groups[2].Value.Trim();

		// Find the parameter list by matching nested parentheses
		int parenStart = match.Index + match.Length - 1; // Position of opening '('
		int parenEnd = FindMatchingClose(fullParam, parenStart, '(', ')');
		if (parenEnd < 0) return null; // Malformed

		string paramsStr = fullParam[(parenStart + 1)..parenEnd].Trim();

		var funcPtr = new SKFunction {
			Name       = name,
			ReturnType = ParseType(returnTypeStr),
			Parameters = string.IsNullOrWhiteSpace(paramsStr) || paramsStr == "void"
				? new List<SKParameter>()
				: ParseParameters(paramsStr),
		};

		return new SKParameter {
			Name         = name,
			NameFlagless = ExtractCleanName(name),
			Type         = new SKType { Name = "callback", FunctionPtr = funcPtr },
			DefaultValue = defaultValue,
		};
	}

	static void ExtractParamSemantics(SKParameter param) {
		string name = param.Name;

		// Check pass type semantics: in_, out_, ref_
		// These can appear at start or after opt_ prefix (e.g., opt_ref_scroll)
		if (name.StartsWith("in_") || name.Contains("_in_") || name.StartsWith("opt_in_")) {
			param.PassType = SKPassType.In;
		} else if (name.StartsWith("out_") || name.Contains("_out_") || name.StartsWith("opt_out_")) {
			param.PassType = SKPassType.Out;
		} else if (name.StartsWith("ref_") || name.Contains("_ref_") || name.StartsWith("opt_ref_")) {
			param.PassType = SKPassType.Ref;
		}

		// Check for opt_ prefix (marks optional parameters)
		if (name.StartsWith("opt_") || name.Contains("_opt_")) {
			param.Type.IsOptional = true;
		}

		// Check for arr_ in name (in_arr_, out_arr_, arr_)
		if (name.Contains("arr_") || name.Contains("_arr")) {
			param.IsArray = true;
		}

		// Check suffixes for string encoding
		if (name.EndsWith("_utf8") || name.Contains("_utf8_")) {
			param.StringType = SKStringType.UTF8;
		} else if (name.EndsWith("_utf16") || name.Contains("_utf16_")) {
			param.StringType = SKStringType.UTF16;
		} else if (name.EndsWith("_ascii") || name.Contains("_ascii_")) {
			param.StringType = SKStringType.ASCII;
		}
	}

	static string ExtractCleanName(string name) {
		// Remove known prefixes: in_, out_, ref_, opt_, in_arr_, out_arr_
		string clean = name;

		// Remove leading prefixes
		foreach (var prefix in new[] { "in_arr_", "out_arr_", "ref_arr_", "in_", "out_", "ref_", "opt_" }) {
			if (clean.StartsWith(prefix)) {
				clean = clean[prefix.Length..];
				break;
			}
		}

		// Remove trailing suffixes
		foreach (var suffix in new[] { "_utf8", "_utf16", "_ascii", "_ptr" }) {
			if (clean.EndsWith(suffix)) {
				clean = clean[..^suffix.Length];
				break;
			}
		}

		return clean;
	}

	// ─────────────────────────────────────────────────────────────
	// Constant Parsing
	// ─────────────────────────────────────────────────────────────

	static SKConstant? ParseConstant(Match match) {
		string typeStr  = match.Groups[1].Value.Trim();
		string name     = match.Groups[2].Value.Trim();
		string valueStr = match.Groups[3].Value.Trim();

		return new SKConstant {
			Name      = name,
			Type      = ParseType(typeStr),
			ValueExpr = valueStr,
		};
	}

	// ─────────────────────────────────────────────────────────────
	// Type Parsing
	// ─────────────────────────────────────────────────────────────

	static SKType ParseType(string typeStr) {
		var type = new SKType();

		typeStr = typeStr.Trim();

		// Check for const
		if (typeStr.StartsWith("const ")) {
			type.IsConst = true;
			typeStr = typeStr[6..].Trim();
		}
		// Const can also appear after the type
		if (typeStr.EndsWith(" const")) {
			type.IsConst = true;
			typeStr = typeStr[..^6].Trim();
		}

		// Count and remove pointer symbols
		while (typeStr.EndsWith('*')) {
			type.PointerLevel++;
			typeStr = typeStr[..^1].Trim();
		}

		// Handle array notation: type[N]
		var arrayMatch = TypeArrayRegex.Match(typeStr);
		if (arrayMatch.Success) {
			typeStr = arrayMatch.Groups[1].Value.Trim();
			type.ArraySize1 = int.Parse(arrayMatch.Groups[2].Value);
			if (arrayMatch.Groups[3].Success) {
				type.ArraySize2 = int.Parse(arrayMatch.Groups[3].Value);
			}
		}

		type.Name = typeStr;
		return type;
	}

	// ─────────────────────────────────────────────────────────────
	// Bitflag Detection
	// ─────────────────────────────────────────────────────────────

	static void CollectBitflagMarkers(string content, HashSet<string> bitflagEnums) {
		foreach (Match match in BitflagRegex.Matches(content)) {
			bitflagEnums.Add(match.Groups[1].Value);
		}
	}

	static void DetectBitflags(SKHeaderData data, HashSet<string> bitflagEnums) {
		foreach (var e in data.Enums) {
			// Check if any value uses bit-shift operators
			bool hasBitShift = e.Items.Any(item =>
				item.ValueExpr != null && item.ValueExpr.Contains("<<"));

			// Check if enum name has SK_MakeFlag marker
			bool hasFlagMarker = bitflagEnums.Contains(e.Name);

			e.IsBitflag = hasBitShift || hasFlagMarker;
		}
	}

	// ─────────────────────────────────────────────────────────────
	// Module Building
	// ─────────────────────────────────────────────────────────────

	static void BuildModules(SKHeaderData data, List<ModuleOverride> overrides) {
		// Add modules for opaque types (assets)
		foreach (var opaqueName in data.OpaqueTypes) {
			// Check if we already have this module from struct parsing
			if (data.Modules.Any(m => m.Name == opaqueName)) continue;

			// Remove _t suffix for module name
			string moduleName = opaqueName.EndsWith("_t")
				? opaqueName[..^2]
				: opaqueName;

			data.Modules.Add(new SKModule {
				Name   = moduleName,
				Prefix = moduleName + "_",
				Type   = SKModuleType.Asset,
			});
		}

		// Apply module overrides
		foreach (var over in overrides) {
			var existing = data.Modules.FirstOrDefault(m => m.Name == over.Name);
			if (existing != null) {
				existing.Prefix = over.Prefix;
			} else {
				data.Modules.Add(new SKModule {
					Name   = over.Name,
					Prefix = over.Prefix,
					Type   = SKModuleType.None,
				});
			}
		}
	}

	// ─────────────────────────────────────────────────────────────
	// Module Association
	// ─────────────────────────────────────────────────────────────

	/// <summary>
	/// Associates items with modules by matching names to prefixes.
	/// </summary>
	static void AssociateWithModules<T>(
		IEnumerable<T> items,
		List<SKModule> sortedModules,
		Func<T, string> getName,
		Func<SKModule, T, bool> matches,
		Action<T, SKModule> onMatch)
	{
		foreach (var item in items) {
			string name = getName(item);
			foreach (var module in sortedModules) {
				if (matches(module, item)) {
					onMatch(item, module);
					break;
				}
			}
		}
	}

	static void AssociateFunctionsWithModules(SKHeaderData data, List<SKModule> sortedModules) {
		AssociateWithModules(
			data.Functions,
			sortedModules,
			f => f.Name,
			(m, f) => f.Name.StartsWith(m.Prefix),
			(f, m) => {
				f.Module   = m;
				f.Relation = SKHeaderData.DetermineRelation(m, f);
			});
	}

	static void AssociateEnumsWithModules(SKHeaderData data, List<SKModule> sortedModules) {
		AssociateWithModules(
			data.Enums,
			sortedModules,
			e => e.Name,
			(m, e) => e.Name.StartsWith(m.Prefix) || e.Name.StartsWith(m.Name + "_"),
			(e, m) => e.Module = m);
	}

	static void AssociateConstantsWithModules(SKHeaderData data, List<SKModule> sortedModules) {
		AssociateWithModules(
			data.Constants,
			sortedModules,
			c => c.Name,
			(m, c) => c.Name.StartsWith(m.Prefix),
			(c, m) => c.Module = m);
	}

	// ─────────────────────────────────────────────────────────────
	// Utility Methods
	// ─────────────────────────────────────────────────────────────

	/// <summary>
	/// Finds the matching closing character for nested pairs (braces, parens, etc).
	/// </summary>
	/// <param name="content">The string to search in</param>
	/// <param name="openIndex">Index of the opening character</param>
	/// <param name="open">The opening character (e.g., '(' or '{')</param>
	/// <param name="close">The closing character (e.g., ')' or '}')</param>
	/// <returns>Index of the matching close character, or -1 if not found</returns>
	static int FindMatchingClose(string content, int openIndex, char open, char close) {
		int depth = 1;
		int i = openIndex + 1;
		while (i < content.Length && depth > 0) {
			if (content[i] == open) depth++;
			else if (content[i] == close) depth--;
			i++;
		}
		return depth == 0 ? i - 1 : -1;
	}

	static string CleanComment(string comment) {
		if (string.IsNullOrWhiteSpace(comment)) return "";

		// Remove /* and */
		comment = comment.Trim();
		if (comment.StartsWith("/*")) comment = comment[2..];
		if (comment.EndsWith("*/"))   comment = comment[..^2];

		// Clean up excessive whitespace within each line but preserve newlines
		var lines = comment.Split('\n');
		var cleanLines = new List<string>();
		foreach (var line in lines) {
			// Collapse multiple spaces to one, trim leading/trailing
			string cleanLine = WhitespaceRegex.Replace(line, " ").Trim();
			if (!string.IsNullOrEmpty(cleanLine)) {
				cleanLines.Add(cleanLine);
			}
		}

		return string.Join("\n", cleanLines);
	}

	// ─────────────────────────────────────────────────────────────
	// Module Override File Parsing
	// ─────────────────────────────────────────────────────────────

	public static List<ModuleOverride> LoadModuleOverrides(string filePath) {
		var overrides = new List<ModuleOverride>();

		if (!File.Exists(filePath)) return overrides;

		foreach (var line in File.ReadAllLines(filePath)) {
			var trimmed = line.Trim();
			if (string.IsNullOrEmpty(trimmed) || trimmed.StartsWith('#'))
				continue;

			var parts = trimmed.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries);
			if (parts.Length >= 2) {
				overrides.Add(new ModuleOverride {
					Name   = parts[0],
					Prefix = parts[1],
				});
			}
		}

		return overrides;
	}
}
