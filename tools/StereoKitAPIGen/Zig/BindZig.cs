using System.Text;

namespace StereoKitAPIGen;

/// <summary>
/// Generates basic Zig bindings from parsed header data.
/// Outputs a single StereoKit.zig file with extern declarations.
/// </summary>
public static class BindZig {
	static NameOverrides _overrides = new();
	static HashSet<string> _opaqueTypes = new();

	public static void Bind(SKHeaderData data, string outputFolder, NameOverrides overrides) {
		_overrides   = overrides;
		_opaqueTypes = data.OpaqueTypes;

		var text = new StringBuilder();
		text.AppendLine(FileHeader);

		// Type aliases
		text.AppendLine("pub const bool32 = i32;");
		text.AppendLine("pub const false32: bool32 = 0;");
		text.AppendLine("pub const true32: bool32 = 1;");
		text.AppendLine();

		// Enums
		foreach (var e in data.Enums) {
			BuildEnum(text, e);
			text.AppendLine();
		}

		// Structs with fields
		foreach (var m in data.Modules.Where(m => m.Fields.Count > 0)) {
			BuildStruct(text, m);
			text.AppendLine();
		}

		// Opaque/Asset types
		foreach (var m in data.Modules.Where(m => m.IsAsset)) {
			text.AppendLine($"pub const {NameType(m.Name)} = ?*opaque {{}};");
		}
		text.AppendLine();

		// Extern function declarations
		text.AppendLine("// ─────────────────────────────────────────────────────────────");
		text.AppendLine("// Extern Functions");
		text.AppendLine("// ─────────────────────────────────────────────────────────────");
		text.AppendLine();

		string currentModule = "";
		foreach (var fn in data.Functions) {
			string moduleName = fn.Module?.Name ?? GetModuleFromName(fn.Name);
			if (moduleName != currentModule) {
				if (!string.IsNullOrEmpty(currentModule)) text.AppendLine();
				text.AppendLine($"// {moduleName}");
				currentModule = moduleName;
			}
			BuildExternFunction(text, fn);
		}

		// Ensure output directory exists
		if (!Directory.Exists(outputFolder)) {
			Directory.CreateDirectory(outputFolder);
		}

		File.WriteAllText(Path.Combine(outputFolder, "StereoKit.zig"), text.ToString());
		Console.WriteLine($"Generated StereoKit.zig with {data.Enums.Count} enums, {data.Modules.Count} modules, {data.Functions.Count} functions");
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

	static void BuildEnum(StringBuilder sb, SKEnum e) {
		string enumName = NameType(e.Name);

		// Comment
		if (!string.IsNullOrWhiteSpace(e.Comment)) {
			sb.AppendLine($"/// {e.Comment}");
		}

		sb.AppendLine($"pub const {enumName} = enum(c_int) {{");

		foreach (var item in e.Items) {
			if (!string.IsNullOrWhiteSpace(item.Comment)) {
				sb.AppendLine($"\t/// {item.Comment}");
			}

			string itemName = NameEnumField(item.Name, e.Name);
			if (item.ValueExpr != null) {
				string value = BuildEnumValue(item.ValueExpr, e.Name);
				sb.AppendLine($"\t{itemName} = {value},");
			} else {
				sb.AppendLine($"\t{itemName},");
			}
		}

		sb.AppendLine("};");
	}

	static string BuildEnumValue(string expr, string enumPrefix) {
		// Try to evaluate as integer
		if (long.TryParse(expr, out long val)) return val.ToString();

		// Bit shift
		var shiftMatch = SKParser.BitShiftRegex.Match(expr);
		if (shiftMatch.Success) {
			int baseVal = int.Parse(shiftMatch.Groups[1].Value);
			int shift   = int.Parse(shiftMatch.Groups[2].Value);
			return (baseVal << shift).ToString();
		}

		// For compound expressions, try to evaluate or return as-is
		// This is simplified - complex expressions may need more work
		return expr;
	}

	// ─────────────────────────────────────────────────────────────
	// Struct Generation
	// ─────────────────────────────────────────────────────────────

	static void BuildStruct(StringBuilder sb, SKModule m) {
		string structName = NameType(m.Name);
		string keyword = m.IsUnion ? "union" : "struct";

		if (!string.IsNullOrWhiteSpace(m.Comment)) {
			sb.AppendLine($"/// {m.Comment}");
		}

		sb.AppendLine($"pub const {structName} = extern {keyword} {{");

		foreach (var f in m.Fields) {
			string fieldName = NameField(f.Name);
			string fieldType = MapType(f.Type);
			string defaultVal = GetDefaultValue(f.Type);

			sb.AppendLine($"\t{fieldName}: {fieldType}{defaultVal},");
		}

		sb.AppendLine("};");
	}

	static string GetDefaultValue(SKType type) {
		if (type.PointerLevel > 0) return " = null";

		return type.Name switch {
			"float" or "double"   => " = 0",
			"int8_t" or "int16_t" or "int32_t" or "int64_t"    => " = 0",
			"uint8_t" or "uint16_t" or "uint32_t" or "uint64_t" => " = 0",
			"bool32_t" => " = false32",
			_ => " = .{}",
		};
	}

	// ─────────────────────────────────────────────────────────────
	// Function Generation
	// ─────────────────────────────────────────────────────────────

	static void BuildExternFunction(StringBuilder sb, SKFunction fn) {
		var paramParts = new List<string>();

		foreach (var p in fn.Parameters) {
			string paramName = NameParam(p.NameFlagless);
			string paramType = MapType(p.Type);

			// Handle arrays
			if (p.IsArray || p.Type.ArraySize1 > 0) {
				paramType = $"[*]" + MapType(new SKType { Name = p.Type.Name });
			}

			// Handle pointers for out/ref params
			if (p.PassType == SKPassType.Out || p.PassType == SKPassType.Ref) {
				paramType = $"*{MapType(new SKType { Name = p.Type.Name })}";
			}

			paramParts.Add($"{paramName}: {paramType}");
		}

		string returnType = fn.ReturnType.IsVoid ? "void" : MapType(fn.ReturnType);
		string paramsStr = string.Join(", ", paramParts);

		sb.AppendLine($"pub extern fn {fn.Name}({paramsStr}) {returnType};");
	}

	// ─────────────────────────────────────────────────────────────
	// Type Mapping
	// ─────────────────────────────────────────────────────────────

	static string MapType(SKType type) {
		string baseName = type.Name;

		// Check override
		if (_overrides.TryGet(baseName, out string? mapped)) {
			return mapped;
		}

		// Standard type mappings
		string zigType = baseName switch {
			"int8_t"   => "i8",
			"int16_t"  => "i16",
			"int32_t"  => "i32",
			"int64_t"  => "i64",
			"uint8_t"  => "u8",
			"uint16_t" => "u16",
			"uint32_t" => "u32",
			"uint64_t" => "u64",
			"bool32_t" => "bool32",
			"float"    => "f32",
			"double"   => "f64",
			"void"     => "void",
			"char"     => "u8",
			"char16_t" => "u16",
			"size_t"   => "usize",
			_          => NameType(baseName),
		};

		// Handle opaque types
		if (_opaqueTypes.Contains(type.Name)) {
			zigType = NameType(type.Name.EndsWith("_t") ? type.Name[..^2] : type.Name);
		}

		// Handle pointers
		if (type.PointerLevel > 0) {
			if (type.Name == "void") {
				return "?*anyopaque";
			}
			if (type.Name == "char" && type.IsConst) {
				return "[*:0]const u8"; // Null-terminated string
			}
			string ptr = type.IsConst ? "*const " : "*";
			for (int i = 0; i < type.PointerLevel; i++) {
				zigType = $"?{ptr}{zigType}";
			}
		}

		// Handle fixed arrays
		if (type.ArraySize1 > 0) {
			zigType = $"[{type.ArraySize1}]{zigType}";
		}

		return zigType;
	}

	// ─────────────────────────────────────────────────────────────
	// Naming Utilities
	// ─────────────────────────────────────────────────────────────

	static string NameType(string name) {
		if (_overrides.TryGet(name, out string? custom)) return custom;
		return SnakeToPascal(name);
	}

	static string NameField(string name) {
		if (_overrides.TryGet(name, out string? custom)) return custom;
		return SnakeToCamel(name);
	}

	static string NameParam(string name) {
		if (_overrides.TryGet(name, out string? custom)) return custom;
		// Handle Zig reserved keywords
		return name switch {
			"type"  => "typ",
			"align" => "alignment",
			"error" => "err",
			_       => SnakeToCamel(name),
		};
	}

	static string NameEnumField(string name, string enumPrefix) {
		if (_overrides.TryGet(name, out string? custom)) return custom;

		// Remove enum prefix
		string working = name;
		if (working.StartsWith(enumPrefix)) {
			working = working[enumPrefix.Length..];
		}

		return SnakeToCamel(working);
	}

	static string SnakeToPascal(string name) {
		// Remove trailing _t
		if (name.EndsWith("_t")) name = name[..^2];
		if (name.EndsWith("_"))  name = name[..^1];

		var parts = name.Split('_', StringSplitOptions.RemoveEmptyEntries);
		var sb = new StringBuilder();
		foreach (var part in parts) {
			if (part.Length > 0) {
				sb.Append(char.ToUpper(part[0]));
				if (part.Length > 1) sb.Append(part[1..]);
			}
		}
		return sb.ToString();
	}

	static string SnakeToCamel(string name) {
		var pascal = SnakeToPascal(name);
		if (pascal.Length == 0) return pascal;
		return char.ToLower(pascal[0]) + pascal[1..];
	}

	static string GetModuleFromName(string funcName) {
		int idx = funcName.IndexOf('_');
		return idx > 0 ? funcName[..idx] : funcName;
	}
}
