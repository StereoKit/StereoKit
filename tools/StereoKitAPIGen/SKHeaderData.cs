namespace StereoKitAPIGen;

// ─────────────────────────────────────────────────────────────
// Semantic Enums
// ─────────────────────────────────────────────────────────────

/// <summary>String encoding hint derived from parameter name suffixes.</summary>
public enum SKStringType { None, ASCII, UTF8, UTF16 }

/// <summary>Parameter passing mode derived from in_/out_/ref_ prefixes.</summary>
public enum SKPassType { None, In, Out, Ref }

/// <summary>Classification of a module (struct, union, or opaque asset handle).</summary>
public enum SKModuleType { None, Struct, Union, Asset }

/// <summary>Relationship between a function and its associated module.</summary>
public enum SKFunctionRelation {
	/// <summary>No module relationship (standalone function).</summary>
	Static,
	/// <summary>First parameter is the module type (instance method).</summary>
	Instance,
	/// <summary>Returns the module type (factory/creation function).</summary>
	Creation
}

// ─────────────────────────────────────────────────────────────
// Type Representation
// ─────────────────────────────────────────────────────────────

/// <summary>Represents a C type with pointer levels, array dimensions, and qualifiers.</summary>
public class SKType {
	/// <summary>Base type name (e.g., "int32_t", "mesh_t", "vec3").</summary>
	public string Name { get; set; } = "";

	/// <summary>Number of pointer indirections (0, 1, 2...).</summary>
	public int PointerLevel { get; set; }

	/// <summary>First array dimension size (0 = not an array).</summary>
	public int ArraySize1 { get; set; }

	/// <summary>Second array dimension size for 2D arrays (0 = not 2D).</summary>
	public int ArraySize2 { get; set; }

	/// <summary>Whether the type is const-qualified.</summary>
	public bool IsConst { get; set; }

	/// <summary>Whether the type is optional/nullable (typically for pointers).</summary>
	public bool IsOptional { get; set; }

	/// <summary>For function pointer types, the function signature.</summary>
	public SKFunction? FunctionPtr { get; set; }

	public bool IsVoid     => Name == "void" && PointerLevel == 0;
	public bool IsPointer  => PointerLevel > 0;
	public bool IsArray    => ArraySize1 > 0;
	public bool IsCallback => FunctionPtr != null;

	public override string ToString() {
		string result = IsConst ? "const " : "";
		result += Name;
		if (ArraySize1 > 0) result += $"[{ArraySize1}]";
		if (ArraySize2 > 0) result += $"[{ArraySize2}]";
		result += new string('*', PointerLevel);
		return result;
	}
}

// ─────────────────────────────────────────────────────────────
// Parameters
// ─────────────────────────────────────────────────────────────

/// <summary>Represents a function parameter with semantic metadata extracted from naming conventions.</summary>
public class SKParameter {
	/// <summary>Original parameter name including prefixes (e.g., "out_arr_vertices").</summary>
	public string Name { get; set; } = "";

	/// <summary>Parameter name with semantic prefixes stripped (e.g., "vertices").</summary>
	public string NameFlagless { get; set; } = "";

	/// <summary>The parameter's type.</summary>
	public SKType Type { get; set; } = new();

	/// <summary>Passing mode derived from in_/out_/ref_ prefix.</summary>
	public SKPassType PassType { get; set; }

	/// <summary>String encoding derived from _utf8/_utf16/_ascii suffix.</summary>
	public SKStringType StringType { get; set; }

	/// <summary>Whether this is an array parameter (arr_ prefix).</summary>
	public bool IsArray { get; set; }

	/// <summary>Whether this came from sk_ref_arr() - a double pointer returning array pointer (C# out IntPtr).</summary>
	public bool IsRefArr { get; set; }

	/// <summary>Default value from sk_default(), if any.</summary>
	public string? DefaultValue { get; set; }

	/// <summary>Documentation comment for this parameter.</summary>
	public string Comment { get; set; } = "";

	public override string ToString() => $"{Type} {Name}";
}

// ─────────────────────────────────────────────────────────────
// Functions
// ─────────────────────────────────────────────────────────────

/// <summary>Represents a C function declaration.</summary>
public class SKFunction {
	/// <summary>Function name (e.g., "mesh_create", "sk_init").</summary>
	public string Name { get; set; } = "";

	/// <summary>Return type.</summary>
	public SKType ReturnType { get; set; } = new();

	/// <summary>Function parameters.</summary>
	public List<SKParameter> Parameters { get; set; } = new();

	/// <summary>Documentation comment.</summary>
	public string Comment { get; set; } = "";

	/// <summary>Whether this function is marked as deprecated.</summary>
	public bool IsDeprecated { get; set; }

	/// <summary>Deprecation message if deprecated.</summary>
	public string? DeprecationMsg { get; set; }

	/// <summary>Associated module (assigned during module association phase).</summary>
	public SKModule? Module { get; set; }

	/// <summary>Relationship to the associated module.</summary>
	public SKFunctionRelation Relation { get; set; } = SKFunctionRelation.Static;

	public override string ToString() => $"{ReturnType} {Name}({string.Join(", ", Parameters)})";
}

// ─────────────────────────────────────────────────────────────
// Enums
// ─────────────────────────────────────────────────────────────

/// <summary>Represents a single enum value.</summary>
public class SKEnumItem {
	/// <summary>Value name (e.g., "display_mode_flatscreen").</summary>
	public string Name { get; set; } = "";

	/// <summary>Original value expression as a string (e.g., "1 << 2", "0").</summary>
	public string? ValueExpr { get; set; }

	/// <summary>Evaluated numeric value if the expression is simple.</summary>
	public long? ValueNumeric { get; set; }

	/// <summary>Documentation comment for this value.</summary>
	public string Comment { get; set; } = "";

	public override string ToString() => ValueExpr != null ? $"{Name} = {ValueExpr}" : Name;
}

/// <summary>Represents a C enum definition.</summary>
public class SKEnum {
	/// <summary>Enum type name (e.g., "display_mode_").</summary>
	public string Name { get; set; } = "";

	/// <summary>Enum values.</summary>
	public List<SKEnumItem> Items { get; set; } = new();

	/// <summary>Whether this enum is a bitflag (detected via SK_MakeFlag or << operators).</summary>
	public bool IsBitflag { get; set; }

	/// <summary>Documentation comment.</summary>
	public string Comment { get; set; } = "";

	/// <summary>Associated module if the enum prefix matches a module.</summary>
	public SKModule? Module { get; set; }

	public override string ToString() => $"enum {Name} ({Items.Count} values{(IsBitflag ? ", bitflag" : "")})";
}

// ─────────────────────────────────────────────────────────────
// Structs/Unions
// ─────────────────────────────────────────────────────────────

/// <summary>Represents a struct or union field.</summary>
public class SKField {
	/// <summary>Field name.</summary>
	public string Name { get; set; } = "";

	/// <summary>Field type.</summary>
	public SKType Type { get; set; } = new();

	/// <summary>Documentation comment.</summary>
	public string Comment { get; set; } = "";

	public override string ToString() => $"{Type} {Name}";
}

// ─────────────────────────────────────────────────────────────
// Modules (Structs, Unions, Assets, or Function Groupings)
// ─────────────────────────────────────────────────────────────

/// <summary>
/// Represents a module, which groups related types and functions.
/// A module can be a struct, union, opaque asset handle, or just a function grouping.
/// </summary>
public class SKModule {
	/// <summary>Module name (e.g., "mesh", "vec3", "model_node").</summary>
	public string Name { get; set; } = "";

	/// <summary>Function prefix for this module (e.g., "mesh_", "vec3_", "model_node_").</summary>
	public string Prefix { get; set; } = "";

	/// <summary>Module classification.</summary>
	public SKModuleType Type { get; set; }

	/// <summary>Fields for struct/union modules.</summary>
	public List<SKField> Fields { get; set; } = new();

	/// <summary>Documentation comment.</summary>
	public string Comment { get; set; } = "";

	public bool IsAsset  => Type == SKModuleType.Asset;
	public bool IsStruct => Type == SKModuleType.Struct;
	public bool IsUnion  => Type == SKModuleType.Union;

	public override string ToString() => $"module {Name} ({Type})";
}

// ─────────────────────────────────────────────────────────────
// Constants
// ─────────────────────────────────────────────────────────────

/// <summary>Represents a global constant defined with SK_CONST.</summary>
public class SKConstant {
	/// <summary>Constant name.</summary>
	public string Name { get; set; } = "";

	/// <summary>Constant type.</summary>
	public SKType Type { get; set; } = new();

	/// <summary>Value expression as a string.</summary>
	public string ValueExpr { get; set; } = "";

	/// <summary>Documentation comment.</summary>
	public string Comment { get; set; } = "";

	/// <summary>Associated module if the constant prefix matches.</summary>
	public SKModule? Module { get; set; }

	public override string ToString() => $"const {Type} {Name} = {ValueExpr}";
}

// ─────────────────────────────────────────────────────────────
// Root Container
// ─────────────────────────────────────────────────────────────

/// <summary>Root container for all parsed header data.</summary>
public class SKHeaderData {
	/// <summary>All modules (structs, unions, assets, function groupings).</summary>
	public List<SKModule> Modules { get; set; } = new();

	/// <summary>All enum definitions.</summary>
	public List<SKEnum> Enums { get; set; } = new();

	/// <summary>All function declarations.</summary>
	public List<SKFunction> Functions { get; set; } = new();

	/// <summary>All constants.</summary>
	public List<SKConstant> Constants { get; set; } = new();

	/// <summary>Set of opaque type names from SK_DeclarePrivateType.</summary>
	public HashSet<string> OpaqueTypes { get; set; } = new();

	/// <summary>Get all functions associated with a module.</summary>
	public IEnumerable<SKFunction> GetModuleFunctions(SKModule module)
		=> Functions.Where(f => f.Module == module);

	/// <summary>Get all constants associated with a module.</summary>
	public IEnumerable<SKConstant> GetModuleConstants(SKModule module)
		=> Constants.Where(c => c.Module == module);

	/// <summary>Determine the relationship between a function and a module.</summary>
	public static SKFunctionRelation DetermineRelation(SKModule module, SKFunction fn) {
		// If return type matches the module's type name, it's a creation function
		string typeName = module.Name + "_t";
		if (module.IsAsset && fn.ReturnType.Name == typeName)
			return SKFunctionRelation.Creation;

		// If first parameter is the module type, it's an instance method
		if (fn.Parameters.Count > 0 && fn.Parameters[0].Type.Name == typeName)
			return SKFunctionRelation.Instance;

		// Otherwise it's a static function
		return SKFunctionRelation.Static;
	}

	public override string ToString()
		=> $"SKHeaderData: {Modules.Count} modules, {Enums.Count} enums, {Functions.Count} functions, {Constants.Count} constants";
}
