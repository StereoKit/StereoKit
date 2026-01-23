using System.Text;

namespace StereoKitAPIGen;

/// <summary>
/// Generates C# bindings from parsed header data.
/// Outputs NativeEnums.cs and NativeAPI.cs files.
/// </summary>
public static class BindCSharp {
	static NameOverrides _overrides = new();
	static HashSet<string> _opaqueTypes = new();

	public static void Bind(SKHeaderData data, string outputFolder, NameOverrides overrides) {
		_overrides   = overrides;
		_opaqueTypes = data.OpaqueTypes;

		// Generate enums
		var enumText = new StringBuilder();
		enumText.AppendLine(FileHeader);
		enumText.AppendLine("using System;");
		enumText.AppendLine();
		enumText.AppendLine("namespace StereoKit");
		enumText.AppendLine("{");

		foreach (var e in data.Enums) {
			BuildEnum(enumText, e, 1);
			enumText.AppendLine();
		}

		enumText.AppendLine("}");
		File.WriteAllText(Path.Combine(outputFolder, "NativeEnums.cs"), enumText.ToString());
		Console.WriteLine($"Generated NativeEnums.cs with {data.Enums.Count} enums");

		// Generate P/Invoke bindings
		var fnText       = new StringBuilder();
		var delegates    = new Dictionary<string, string>();
		var moduleGroups = GroupFunctionsByModule(data.Functions);

		foreach (var (moduleName, functions) in moduleGroups) {
			fnText.AppendLine();
			fnText.AppendLine("\t\t///////////////////////////////////////////");
			fnText.AppendLine();
			foreach (var fn in functions) {
				BuildFunction(fnText, fn, delegates, "\t\t");
			}
		}

		var fnFileText = new StringBuilder();
		fnFileText.AppendLine(FileHeader);
		fnFileText.AppendLine("using System;");
		fnFileText.AppendLine("using System.Runtime.InteropServices;");
		fnFileText.AppendLine();
		fnFileText.AppendLine("namespace StereoKit");
		fnFileText.AppendLine("{");

		// Write delegate definitions at namespace level (public, for use by wrapper code)
		foreach (var d in delegates.Values) {
			if (!string.IsNullOrEmpty(d)) {
				fnFileText.AppendLine($"\t[UnmanagedFunctionPointer(CallingConvention.Cdecl)] {d}");
			}
		}
		if (delegates.Count > 0) fnFileText.AppendLine();

		fnFileText.AppendLine("\tinternal static partial class NativeAPI");
		fnFileText.AppendLine("\t{");
		fnFileText.AppendLine("\t\tconst string            dll  = \"StereoKitC\";");
		fnFileText.AppendLine("\t\tconst CharSet           cSet = CharSet.Ansi;");
		fnFileText.AppendLine("\t\tconst CallingConvention call = CallingConvention.Cdecl;");

		fnFileText.Append(fnText);
		fnFileText.AppendLine("\t}");
		fnFileText.AppendLine("}");

		File.WriteAllText(Path.Combine(outputFolder, "NativeAPI.cs"), fnFileText.ToString());
		Console.WriteLine($"Generated NativeAPI.cs with {data.Functions.Count} functions");

		// Generate structs
		var structText = new StringBuilder();
		structText.AppendLine(FileHeader);
		structText.AppendLine("using System.Runtime.InteropServices;");
		structText.AppendLine();
		structText.AppendLine("namespace StereoKit");
		structText.AppendLine("{");

		int structCount = 0;
		foreach (var module in data.Modules) {
			if (module.Type == SKModuleType.Struct || module.Type == SKModuleType.Union) {
				if (BuildStruct(structText, module, 1)) {
					structText.AppendLine();
					structCount++;
				}
			}
		}

		structText.AppendLine("}");
		File.WriteAllText(Path.Combine(outputFolder, "NativeTypes.cs"), structText.ToString());
		Console.WriteLine($"Generated NativeTypes.cs with {structCount} structs");
	}

	// ─────────────────────────────────────────────────────────────
	// File Header
	// ─────────────────────────────────────────────────────────────

	const string FileHeader = @"// This is a generated file based on stereokit.h! Please don't modify it
// directly :) Instead, modify the header file, and run the StereoKitAPIGen
// project.
";

	// ─────────────────────────────────────────────────────────────
	// Enum Generation
	// ─────────────────────────────────────────────────────────────

	static void BuildEnum(StringBuilder sb, SKEnum e, int indent) {
		string prefix = new string('\t', indent);
		string enumName = SnakeToPascal(e.Name);

		// Skip types marked with @noimpl in overrides (hand-written elsewhere)
		if (_overrides.ShouldSkipImpl(e.Name)) return;

		string valuePrefix = e.Name; // e.g., "display_mode_"

		// XML doc comment
		if (!string.IsNullOrWhiteSpace(e.Comment)) {
			BuildXmlComment(sb, e.Comment, indent);
		}

		// [Flags] attribute
		if (e.IsBitflag) {
			sb.AppendLine($"{prefix}[Flags]");
		}

		sb.AppendLine($"{prefix}public enum {enumName} {{");

		foreach (var item in e.Items) {
			// Extract obsolete info and clean comment
			string cleanComment = ExtractObsolete(item.Comment, out string? obsoleteMessage);

			// XML doc comment for item
			if (!string.IsNullOrWhiteSpace(cleanComment)) {
				BuildXmlComment(sb, cleanComment, indent + 1);
			}

			// [Obsolete] attribute if found in comment
			if (obsoleteMessage != null) {
				if (string.IsNullOrEmpty(obsoleteMessage)) {
					sb.AppendLine($"{prefix}\t[Obsolete]");
				} else {
					sb.AppendLine($"{prefix}\t[Obsolete(\"{obsoleteMessage}\")]");
				}
			}

			string itemName = SnakeToPascal(item.Name, valuePrefix.Length);
			string valueStr = BuildEnumValueExpr(item.ValueExpr, valuePrefix);

			if (string.IsNullOrEmpty(valueStr)) {
				sb.AppendLine($"{prefix}\t{itemName},");
			} else {
				sb.AppendLine($"{prefix}\t{itemName,-12} = {valueStr},");
			}
		}

		sb.AppendLine($"{prefix}}}");
	}

	// ─────────────────────────────────────────────────────────────
	// Struct Generation
	// ─────────────────────────────────────────────────────────────

	static bool BuildStruct(StringBuilder sb, SKModule module, int indent) {
		string prefix = new string('\t', indent);
		string structName = SnakeToPascal(module.Name);

		// Skip structs marked with @noimpl in overrides (hand-written elsewhere)
		if (_overrides.ShouldSkipImpl(module.Name)) return false;

		// Check for complex fields that we can't auto-generate
		foreach (var field in module.Fields) {
			// Skip structs with pointers (need special handling)
			if (field.Type.PointerLevel > 0) return false;
			// Skip structs with function pointers
			if (field.Type.FunctionPtr != null) return false;
		}

		// XML doc comment
		if (!string.IsNullOrWhiteSpace(module.Comment)) {
			BuildXmlComment(sb, module.Comment, indent);
		}

		// StructLayout attribute
		string layoutKind = module.Type == SKModuleType.Union ? "Explicit" : "Sequential";
		sb.AppendLine($"{prefix}[StructLayout(LayoutKind.{layoutKind})]");
		sb.AppendLine($"{prefix}public partial struct {structName}");
		sb.AppendLine($"{prefix}{{");

		int fieldOffset = 0;
		foreach (var field in module.Fields) {
			// XML doc comment for field
			if (!string.IsNullOrWhiteSpace(field.Comment)) {
				BuildXmlComment(sb, field.Comment, indent + 1);
			}

			// Union fields need FieldOffset
			if (module.Type == SKModuleType.Union) {
				sb.AppendLine($"{prefix}\t[FieldOffset({fieldOffset})]");
			}

			string fieldType = MapFieldType(field.Type);
			string fieldName = SnakeToCamel(field.Name);

			// Handle fixed-size arrays
			if (field.Type.ArraySize1 > 0) {
				// Use MarshalAs for fixed arrays
				sb.AppendLine($"{prefix}\t[MarshalAs(UnmanagedType.ByValArray, SizeConst = {field.Type.ArraySize1})]");
				sb.AppendLine($"{prefix}\tpublic {fieldType}[] {fieldName};");
			} else if (field.Type.Name == "bool32_t") {
				// bool32_t marshals as 4-byte Windows BOOL
				sb.AppendLine($"{prefix}\t[MarshalAs(UnmanagedType.Bool)] public bool {fieldName};");
			} else {
				sb.AppendLine($"{prefix}\tpublic {fieldType} {fieldName};");
			}
		}

		sb.AppendLine($"{prefix}}}");
		return true;
	}

	static string MapFieldType(SKType type) {
		string baseName = type.Name;

		// Check for override first - but _SKIP_ means use the default PascalCase name
		if (_overrides.TryGet(baseName, out string? mapped) && mapped != "_SKIP_") {
			return mapped;
		}

		// Try common primitives first
		var primitive = MapPrimitiveType(baseName);
		if (primitive != null) return primitive;

		// Field-specific mappings
		return baseName switch {
			"char" => "byte",
			_      => SnakeToPascalDirect(baseName),
		};
	}

	/// <summary>
	/// Extracts an obsolete message from a comment and returns the cleaned comment.
	/// Looks for lines starting with "Obsolete:" or just "Obsolete" (case-insensitive).
	/// </summary>
	/// <param name="comment">The original comment</param>
	/// <param name="obsoleteMessage">The extracted message (null if not found, empty string if just "Obsolete")</param>
	/// <returns>The comment with the obsolete line removed</returns>
	static string ExtractObsolete(string comment, out string? obsoleteMessage) {
		obsoleteMessage = null;
		if (string.IsNullOrWhiteSpace(comment)) return comment;

		var lines = comment.Split('\n');
		var resultLines = new List<string>();

		foreach (var line in lines) {
			string trimmed = line.Trim();
			if (trimmed.StartsWith("Obsolete:", StringComparison.OrdinalIgnoreCase)) {
				// Extract message after "Obsolete:"
				obsoleteMessage = trimmed.Substring(9).Trim();
			} else if (trimmed.Equals("Obsolete", StringComparison.OrdinalIgnoreCase)) {
				// Just "Obsolete" with no message
				obsoleteMessage = "";
			} else {
				resultLines.Add(line);
			}
		}

		return string.Join('\n', resultLines);
	}

	static void BuildXmlComment(StringBuilder sb, string comment, int indent) {
		string prefix = new string('\t', indent) + "/// ";

		// Escape XML special characters
		comment = comment
			.Replace("&", "&amp;")
			.Replace("<", "&lt;")
			.Replace(">", "&gt;");

		// Split into lines and wrap with <summary>
		var lines = comment.Split('\n', StringSplitOptions.RemoveEmptyEntries);

		if (lines.Length == 1) {
			sb.AppendLine($"{prefix}<summary>{lines[0].Trim()}</summary>");
		} else {
			for (int i = 0; i < lines.Length; i++) {
				string line = lines[i].Trim();
				if (i == 0) {
					sb.AppendLine($"{prefix}<summary>{line}");
				} else if (i == lines.Length - 1) {
					sb.AppendLine($"{prefix}{line}</summary>");
				} else {
					sb.AppendLine($"{prefix}{line}");
				}
			}
		}
	}

	static string BuildEnumValueExpr(string? expr, string valuePrefix) {
		if (string.IsNullOrWhiteSpace(expr)) return "";

		// Simple numeric value
		if (int.TryParse(expr, out _)) return expr;

		// Hex value
		if (expr.StartsWith("0x", StringComparison.OrdinalIgnoreCase)) return expr;

		// Bit shift: 1 << N
		if (expr.Contains("<<")) return expr.Trim();

		// Reference to other enum values: convert snake_case to PascalCase
		// e.g., "display_blend_additive | display_blend_blend" -> "Additive | Blend"
		// Handle operators including ~ (bitwise not)
		var parts = expr.Split(new[] { '|', '+', '-', '*', '/', '&', '~', ' ' }, StringSplitOptions.RemoveEmptyEntries);
		string result = expr;
		foreach (var part in parts) {
			string cleanPart = part.Trim();
			if (cleanPart.StartsWith(valuePrefix)) {
				string newName = SnakeToPascal(cleanPart, valuePrefix.Length);
				result = result.Replace(cleanPart, newName);
			}
		}

		return result.Trim();
	}

	// ─────────────────────────────────────────────────────────────
	// Function Generation
	// ─────────────────────────────────────────────────────────────

	static void BuildFunction(StringBuilder sb, SKFunction fn, Dictionary<string, string> delegates, string indent) {
		var returnType = MapType(fn.ReturnType, "", delegates, isReturn: true);
		bool returnsBool = fn.ReturnType.Name == "bool32_t";

		// Build return type MarshalAs if needed
		// NOTE: Do NOT add MarshalAs for string returns! LPUTF8Str/LPWStr tells the
		// marshaller to FREE the returned pointer, but StereoKit returns pointers to
		// internal static strings. Default behavior copies without freeing.
		if (returnsBool) {
			sb.AppendLine($"{indent}[return: MarshalAs(UnmanagedType.Bool)]");
		}

		// Build parameters
		var paramParts = new List<string>();
		SKStringType textType = SKStringType.None;

		foreach (var p in fn.Parameters) {
			var (typeStr, paramTextType) = BuildParameter(p, delegates);
			if (paramTextType != SKStringType.None) textType = paramTextType;

			// Escape reserved C# keywords
			string paramName = EscapeKeyword(p.Name);

			// Skip default values in C# bindings (IR preserves them for other languages)
			paramParts.Add($"{typeStr} {paramName}");
		}

		// Determine CharSet
		string charSet = textType switch {
			SKStringType.UTF16 => ", CharSet = CharSet.Unicode",
			_                  => ", CharSet = cSet",
		};

		// Build the DllImport line
		string returnTypeStr = returnsBool ? "bool" : returnType;
		string paramsStr = string.Join(", ", paramParts);
		sb.AppendLine($"{indent}[DllImport(dll{charSet}, CallingConvention = call)] public static extern {returnTypeStr,-12} {fn.Name}({paramsStr});");
	}

	static (string typeStr, SKStringType textType) BuildParameter(SKParameter p, Dictionary<string, string> delegates) {
		var type = p.Type;
		var textType = p.StringType;
		var passType = p.PassType;

		// Handle function pointers
		if (type.FunctionPtr != null) {
			string delegateName = BuildDelegate(type.FunctionPtr, p.Name, delegates);
			return ($"[MarshalAs(UnmanagedType.FunctionPtr)] {delegateName}", SKStringType.None);
		}

		// Handle bool32_t - use bool with MarshalAs, including ref/out/in prefix
		// Note: MarshalAs must come before ref/out/in for valid C# syntax
		if (type.Name == "bool32_t") {
			string boolPrefix = passType switch {
				SKPassType.In  => "in ",
				SKPassType.Out => "out ",
				SKPassType.Ref => "ref ",
				_              => "",
			};
			return ($"[MarshalAs(UnmanagedType.Bool)] {boolPrefix}bool", SKStringType.None);
		}

		string typeName = MapType(type, p.Name, delegates, isReturn: false);

		// Handle char* and char** strings (UTF-8)
		// char* (single pointer) -> string with LPUTF8Str marshalling
		// char** (double pointer) -> string[] (array of strings) for input, out IntPtr for output
		if (type.Name == "char" && type.PointerLevel > 0) {
			// Double pointer char** - usually array of strings
			if (type.PointerLevel >= 2) {
				if (passType == SKPassType.Out) {
					return ("out IntPtr", textType);
				}
				// const char** is typically an array of strings (input)
				// Even without arr_ prefix, treat as string array for const char**
				return ("[In] string[]", textType);
			}
			// Single pointer char*
			if (passType == SKPassType.Out) {
				return ("out IntPtr", textType);
			} else if (passType == SKPassType.Ref) {
				return ("ref IntPtr", textType);
			}
			return ("[MarshalAs(UnmanagedType.LPUTF8Str)] string", textType);
		}

		// Handle char16_t* strings (UTF-16)
		// char16_t* (single pointer) -> string with LPWStr marshalling
		// char16_t** would need special handling but doesn't exist in the API
		if (type.Name == "char16_t" && type.PointerLevel > 0) {
			// For writable buffers (no const, not out), use StringBuilder
			// These are typically input/output buffers for text input fields
			if (!type.IsConst && passType != SKPassType.Out) {
				return ("[MarshalAs(UnmanagedType.LPWStr)] System.Text.StringBuilder", SKStringType.UTF16);
			}
			// Single pointer const char16_t* - input string
			if (passType == SKPassType.Out) {
				return ("out IntPtr", SKStringType.UTF16);
			} else if (passType == SKPassType.Ref) {
				return ("ref IntPtr", SKStringType.UTF16);
			}
			return ("[MarshalAs(UnmanagedType.LPWStr)] string", SKStringType.UTF16);
		}

		// Handle sk_ref_arr - double pointer returning array pointer → out IntPtr
		if (p.IsRefArr) {
			string keyword = passType switch {
				SKPassType.Out => "out ",
				SKPassType.Ref => "ref ",
				_              => "",
			};
			return ($"{keyword}IntPtr", textType);
		}

		// Handle arrays
		if (p.IsArray || type.ArraySize1 > 0) {
			if (type.ArraySize1 > 0) {
				// Fixed size array
				return ($"[MarshalAs(UnmanagedType.LPArray, SizeConst = {type.ArraySize1})] {typeName}[]", textType);
			}
			// Variable length array - use [In] or [Out] attribute
			string attr = passType switch {
				SKPassType.Out => "[Out] ",
				SKPassType.Ref => "[In, Out] ",
				_              => "[In] ",
			};
			return ($"{attr}{typeName}[]", textType);
		}

		// For void* (single pointer) buffers, out_/in_/ref_ prefix indicates what
		// the function does with the buffer, not C# parameter passing. Pass IntPtr by value.
		// But void** (double pointer) with out_ prefix means "return a pointer" = out IntPtr
		bool isVoidSinglePointer = type.Name == "void" && type.PointerLevel == 1;
		if (isVoidSinglePointer) {
			return (typeName, textType);
		}

		// Optional pointer parameters (opt_ prefix) need to be IntPtr because
		// C# ref/out can't be null and we need to pass IntPtr.Zero for null.
		// This includes opt_ref_, opt_out_, and opt_ const pointers (like opt_box_transform).
		// The wrapper or NativeAPI.ArrayOverloads.cs provides typed overloads for non-null cases.
		if (type.IsOptional && type.PointerLevel > 0) {
			return ("IntPtr", textType);
		}

		// Handle pass by reference
		string prefix = passType switch {
			SKPassType.In  => "in ",
			SKPassType.Out => "out ",
			SKPassType.Ref => "ref ",
			_              => type.IsConst && type.PointerLevel > 0 ? "in " : "",
		};

		return ($"{prefix}{typeName}", textType);
	}

	static string BuildDelegate(SKFunction fn, string varName, Dictionary<string, string> delegates) {
		// Check for simple Action delegate
		bool isVoidReturn = fn.ReturnType.IsVoid;
		if (isVoidReturn && fn.Parameters.Count == 0) {
			return "Action";
		}

		string delegateName = SnakeToPascal(varName);

		// Check if this delegate should be skipped (hand-written elsewhere)
		if (_overrides.ShouldSkipImpl(delegateName)) {
			// Don't generate definition, but return the name for use in DllImport
			return delegateName;
		}

		// Build delegate definition
		string returnStr = isVoidReturn ? "void" : MapType(fn.ReturnType, "", delegates, isReturn: true);
		var paramParts = new List<string>();
		foreach (var p in fn.Parameters) {
			var (typeStr, _) = BuildParameter(p, delegates);
			paramParts.Add($"{typeStr} {p.Name}");
		}

		string definition = $"internal delegate {returnStr} {delegateName}({string.Join(", ", paramParts)});";
		delegates[delegateName] = definition;

		return delegateName;
	}

	// ─────────────────────────────────────────────────────────────
	// Type Mapping
	// ─────────────────────────────────────────────────────────────

	/// <summary>
	/// Maps common C primitive types to C# types. Returns null for unknown types.
	/// </summary>
	static string? MapPrimitiveType(string typeName) {
		return typeName switch {
			"int8_t"   => "sbyte",
			"int16_t"  => "short",
			"int32_t"  => "int",
			"int64_t"  => "long",
			"uint8_t"  => "byte",
			"uint16_t" => "ushort",
			"uint32_t" => "uint",
			"uint64_t" => "ulong",
			"float"    => "float",
			"double"   => "double",
			"size_t"   => "UIntPtr",
			_          => null,
		};
	}

	static string MapType(SKType type, string varName, Dictionary<string, string> delegates, bool isReturn) {
		string baseName = type.Name;

		// Check for override first
		if (_overrides.TryGet(baseName, out string? mapped)) {
			baseName = mapped;
		} else {
			// Try common primitives first
			var primitive = MapPrimitiveType(baseName);
			if (primitive != null) {
				baseName = primitive;
			} else {
				// Apply type-specific mappings
				baseName = baseName switch {
					"bool32_t" => "int", // Will add MarshalAs for bool separately
					"void"     => "void",
					"char"     => HandleCharType(type, varName, isReturn),
					"char16_t" => "string",
					"char32_t" => "uint",
					_          => SnakeToPascalDirect(baseName),
				};
			}
		}

		// Handle opaque/asset types -> IntPtr
		if (_opaqueTypes.Contains(type.Name)) {
			return "IntPtr";
		}

		// Handle void pointer -> IntPtr (caller uses GCHandle to pin arrays)
		if (baseName == "void" && type.PointerLevel > 0) {
			return "IntPtr";
		}

		// Handle char pointer return types -> IntPtr (wrapper uses NativeHelper.FromUtf8 / Marshal.PtrToString)
		// The marshaller tries to free returned strings which crashes on static native strings.
		// For parameters, handled in BuildParameter with proper MarshalAs.
		if ((type.Name == "char" || type.Name == "char16_t") && type.PointerLevel == 1 && isReturn) {
			return "IntPtr";
		}

		// For return types: struct pointers (non-opaque) -> IntPtr
		// P/Invoke can't auto-marshal pointer returns to structs, wrapper uses Marshal.PtrToStructure
		if (isReturn && type.PointerLevel > 0 && !_opaqueTypes.Contains(type.Name)) {
			return "IntPtr";
		}

		return baseName;
	}

	static string HandleCharType(SKType type, string varName, bool isReturn) {
		if (type.PointerLevel > 0) {
			// UTF-8 byte array
			if (varName.Contains("utf8")) {
				return "byte";
			}
			// Regular string
			return isReturn ? "IntPtr" : "string";
		}
		return "byte"; // Single char -> byte
	}

	// ─────────────────────────────────────────────────────────────
	// Utility Methods
	// ─────────────────────────────────────────────────────────────

	static string SnakeToPascal(string name, int removePrefix = 0) {
		// Check for override
		if (_overrides.TryGet(name, out string? result)) {
			return result;
		}

		return SnakeToPascalDirect(name, removePrefix);
	}

	static string SnakeToPascalDirect(string name, int removePrefix = 0) {
		// Remove prefix and trailing _t
		string working = name[removePrefix..];
		if (working.EndsWith("_t")) {
			working = working[..^2];
		}
		if (working.EndsWith("_")) {
			working = working[..^1];
		}

		// Split and capitalize
		var words = working.Split('_', StringSplitOptions.RemoveEmptyEntries);
		var sb = new StringBuilder();
		foreach (var word in words) {
			if (word.Length > 0) {
				sb.Append(char.ToUpper(word[0]));
				if (word.Length > 1) {
					sb.Append(word[1..]);
				}
			}
		}

		return sb.ToString();
	}

	static string SnakeToCamel(string name) {
		// Convert snake_case to camelCase (first word lowercase, rest capitalized)
		var words = name.Split('_', StringSplitOptions.RemoveEmptyEntries);
		var sb = new StringBuilder();
		for (int i = 0; i < words.Length; i++) {
			var word = words[i];
			if (word.Length > 0) {
				if (i == 0) {
					// First word: all lowercase
					sb.Append(word.ToLower());
				} else {
					// Subsequent words: capitalize first letter
					sb.Append(char.ToUpper(word[0]));
					if (word.Length > 1) {
						sb.Append(word[1..]);
					}
				}
			}
		}
		return sb.ToString();
	}

	static readonly HashSet<string> CSharpKeywords = new() {
		"abstract", "as", "base", "bool", "break", "byte", "case", "catch", "char",
		"checked", "class", "const", "continue", "decimal", "default", "delegate",
		"do", "double", "else", "enum", "event", "explicit", "extern", "false",
		"finally", "fixed", "float", "for", "foreach", "goto", "if", "implicit",
		"in", "int", "interface", "internal", "is", "lock", "long", "namespace",
		"new", "null", "object", "operator", "out", "override", "params", "private",
		"protected", "public", "readonly", "ref", "return", "sbyte", "sealed",
		"short", "sizeof", "stackalloc", "static", "string", "struct", "switch",
		"this", "throw", "true", "try", "typeof", "uint", "ulong", "unchecked",
		"unsafe", "ushort", "using", "virtual", "void", "volatile", "while"
	};

	static string EscapeKeyword(string name) {
		return CSharpKeywords.Contains(name) ? "@" + name : name;
	}

	static List<(string moduleName, List<SKFunction> functions)> GroupFunctionsByModule(List<SKFunction> functions) {
		var groups = new Dictionary<string, List<SKFunction>>();

		foreach (var fn in functions) {
			string moduleName = fn.Module?.Name ?? GetModuleFromName(fn.Name);
			if (!groups.ContainsKey(moduleName)) {
				groups[moduleName] = new List<SKFunction>();
			}
			groups[moduleName].Add(fn);
		}

		return groups.Select(g => (g.Key, g.Value)).ToList();
	}

	static string GetModuleFromName(string funcName) {
		int idx = funcName.IndexOf('_');
		return idx > 0 ? funcName[..idx] : funcName;
	}
}
