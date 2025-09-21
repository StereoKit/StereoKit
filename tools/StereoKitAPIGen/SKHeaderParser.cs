using CppAst;
using System;
using System.Collections.Generic;
using System.Linq;

///////////////////////////////////////////

enum SKStringType
{
	None,
	ASCII,
	UTF8,
	UTF16
}

///////////////////////////////////////////

enum SKPassType
{
	None,
	Ref,
	In,
	Out
}

///////////////////////////////////////////

enum SKFunctionRelation
{
	Instance,
	Static,
	Creation
}

///////////////////////////////////////////

enum SKModuleType
{
	None,
	Struct,
	Union,
	Refcounted,
}

///////////////////////////////////////////

struct SKEnumItem
{
	public string        comment;
	public string        name;
	public CppExpression value;
}

///////////////////////////////////////////

struct SKEnum
{
	public string name;
	public string comment;
	public bool   isBitflag;
	public SKEnumItem[] items;
}

///////////////////////////////////////////

struct SKType
{
	public string name;
	public int    arraySize1;
	public int    arraySize2;
	public bool   isConst;
	public bool   isOptional;
	public int    pointerLvl;
	public Opt<SKFunction> functionPtr;

	public bool IsVoid => name == "void" && arraySize1 == 0 && arraySize2 == 0 && pointerLvl == 0 && isConst == false && functionPtr == null;
}

///////////////////////////////////////////

struct SKField
{
	public string name;
	public SKType type;
}

///////////////////////////////////////////

struct SKParameter
{
	public string name;
	public string nameFlagless;

	public SKType       type;
	public bool         isArrayPtr;
	public SKStringType stringType;
	public SKPassType   passType;

	public CppExpression defaultValue;
}

///////////////////////////////////////////

struct SKConstant
{
	public string name;
	public SKType type;
	public SKModule module;
	public CppExpression value;
}

///////////////////////////////////////////

struct SKFunction
{
	public CppFunction src;
	public SKType        returnType;
	public string        name; // null for function pointers
	public SKModule      module;
	public SKParameter[] parameters;
}

///////////////////////////////////////////

class SKModule
{
	public string       name;
	public string       modulePrefix;
	public bool         isAsset;
	public SKField[]    fields = new SKField[0];
	public SKModuleType type;

	public override bool Equals(object obj) => modulePrefix.Equals(((SKModule)obj).modulePrefix);
	public override int GetHashCode() => modulePrefix.GetHashCode();
}

///////////////////////////////////////////

class SKHeaderData
{
	public SKModule[]   modules;
	public SKEnum[]     enums;
	public SKFunction[] functions;
	public SKConstant[] constants;

	public static SKFunction[] GetModuleFunctions(SKModule module, SKFunction[] functions)
		=> functions.Where(f=> f.module == module).ToArray();
	public static SKConstant[] GetModuleConstants(SKModule module, SKConstant[] functions)
		=> functions.Where(f => f.module == module).ToArray();
	public static SKFunctionRelation GetFunctionRelation(SKModule module, SKFunction function)
	{
		if (function.parameters.Length > 0 && 
		    function.parameters[0].type.name == module.name &&
		    (function.parameters[0].type.pointerLvl > 0 || module.isAsset)) return SKFunctionRelation.Instance;
		else if (function.returnType.name == module.name) return SKFunctionRelation.Creation;
		else return SKFunctionRelation.Static;
	}

	public static bool IsAsset(SKType type, SKModule[] modules)
	{
		foreach (SKModule module in modules)
			if (module.isAsset && type.name == module.name) return true;
		return false;
	}
}

///////////////////////////////////////////

struct ModuleException
{
	public string name;
	public string prefix;
}

///////////////////////////////////////////

static class SKHeaderParser
{
	///////////////////////////////////////////
	
	public static SKHeaderData Parse(List<string> files, ModuleException[] moduleExceptions)
	{
		CppCompilation ast  = CppParser.ParseFiles(files);
		SKHeaderData   data = new();
		data.modules = FindStructModules(ast.Namespaces[0])
			.Union(moduleExceptions.Select(m=> new SKModule { name = m.name, modulePrefix = m.prefix }))
			.Union(FindFunctionModules(ast))
			.OrderBy(m => m.name)
			.ToArray();
		data.enums = ast.Namespaces[0].Enums
			.Select(ParseEnum)
			.OrderBy(e => e.name)
			.ToArray();
		data.functions = FindFunctions(ast)
			.Select(f =>
			{
				SKFunction r = f;
				r.module = data.modules.Aggregate(default(SKModule), (m, c) => f.name.StartsWith(c.modulePrefix) && c.modulePrefix.Length > (m?.modulePrefix.Length ?? 0) ? c : m);
				return r;
			} )
			.ToArray();
		data.constants = ast.Namespaces[0].Fields
			.Select((f) => new SKConstant
			{
				name   = f.Name,
				type   = ParseType(f.Type, f.Name),
				module = data.modules.Aggregate(default(SKModule), (m, c) => f.Name.StartsWith(c.modulePrefix) && c.modulePrefix.Length > (m?.modulePrefix.Length ?? 0) ? c : m),
				value  = f.InitExpression
			})
			.ToArray();

		return data;
	}

	///////////////////////////////////////////

	static SKType ParseType(CppType type, string varName)
	{
		CppType curr   = type;
		SKType  result = new();
		while (curr != null)
		{
			switch (curr)
			{
				case CppPointerType   ptr:  result.pointerLvl += 1; curr = ptr.ElementType; break;
				case CppReferenceType refr: result.pointerLvl += 1; curr = refr.ElementType; break;
				case CppQualifiedType qual: if (qual.Qualifier == CppTypeQualifier.Const) result.isConst = true; curr = qual.ElementType; break;
				case CppArrayType     arr:
					{
						if      (result.arraySize1 == 0) result.arraySize1 = arr.Size;
						else if (result.arraySize2 == 0) result.arraySize2 = arr.Size;
						else throw new Exception("Too many array dimensions!");
						curr = arr.ElementType;
					} break;
				case CppFunctionType fun:
					{
						SKParameter[] parameters = fun.Parameters.Select(ParseParameter).ToArray();
						SKType        returnType = ParseType(fun.ReturnType, "");
						string name = $"fn_{returnType.name}_{(parameters.Length==0?"_void":"")}";
						for (int i = 0; i < parameters.Length; i++)
							name += $"_{(parameters[i].type.pointerLvl == 1 ? "*" : "")}{parameters[i].type.name}";

						result.functionPtr = new Opt<SKFunction>(new SKFunction
						{
							name       = $"fn_{varName}",
							parameters = parameters,
							returnType = returnType
						});
						curr = null;
					} break;
				case CppUnexposedType unexposed:
					{
						// Seems like char16/32_t are somewhat exceptional
						// types here!
						if      (unexposed.Name == "const char16_t") result.name = "char16_t";
						else if (unexposed.Name == "char16_t")       result.name = "char16_t";
						else if (unexposed.Name == "char32_t")       result.name = "char32_t";
						else throw new Exception("Unknown type: " + unexposed.Name);
						curr = null;
					} break;
				default: result.name = curr.GetDisplayName(); curr = null; break;
			}
		}
		return result;
	}

	///////////////////////////////////////////

	static SKType ParseStructType(CppType type)
	{
		SKType result = ParseType(type, type.GetDisplayName());
		if (result.pointerLvl > 0) result.isOptional = true;
		return result;
	}

	///////////////////////////////////////////

	static IEnumerable<SKModule> FindStructModules(CppNamespace ns)
		=> ns.Classes
			.Select(t => new SKModule {
				name         = t.Name.StartsWith("_")? t.Name[1..] : (t.Name == "color128" ? "color" : t.Name),
				modulePrefix = t.Name.EndsWith("_t") ? (t.Name.StartsWith("_")? t.Name[1..^1] : t.Name[..^1]) : (t.Name == "color128" ? "color_" : t.Name + "_"),
				isAsset      = t.Name.StartsWith("_"),
				type         = t.ClassKind == CppClassKind.Union ? SKModuleType.Union : SKModuleType.Struct,
				fields       = t.Fields.Select(f => new SKField {
					name = f.Name,
					type = ParseStructType(f.Type),
				}).ToArray()
			});

	///////////////////////////////////////////

	static IEnumerable<SKModule> FindFunctionModules(CppCompilation ast)
		=> ast.Functions
			.Select(f => f.Name.Split('_')[0])
			.Distinct()
			.Select(n => new SKModule {
				name         = n,
				modulePrefix = n+"_",
				isAsset      = false,
			});

	///////////////////////////////////////////

	static SKEnum ParseEnum(CppEnum e)
		=> new SKEnum {
			name      = e.Name,
			comment   = BuildComment(e.Comment),
			isBitflag = IsBitflag(e),
			items     = e.Items.Select(i => new SKEnumItem
			{
				comment = BuildComment(i.Comment),
				name    = i.Name,
				value   = i.ValueExpression,
			}).ToArray(),
		};

	///////////////////////////////////////////

	static string BuildComment(CppComment comment)
		=> comment?.Children
		.Aggregate("", (str, c) => str + c.Kind switch
			{
				CppCommentKind.Paragraph => c.Children.Aggregate("", (p, s) => p + s.ToString().Trim() + " ") + "\n",
				_ => c.ToString().Trim() + " ",
			})
		.Trim() ?? "";

	///////////////////////////////////////////

	static bool IsBitflag(CppEnum e)
		=> e.Items.Any(i => i.ValueExpression != null && i.ValueExpression.ToString().Contains("<<"));

	///////////////////////////////////////////

	static IEnumerable<SKFunction> FindFunctions(CppCompilation ast)
		=> ast.Functions
			.Select(f => new SKFunction
			{
				src = f,
				name       = f.Name,
				parameters = ParseParameterFix(f.Parameters).ToArray(),
				returnType = ParseType(f.ReturnType, f.Name)
			});

	///////////////////////////////////////////
	
	static IEnumerable<SKParameter> ParseParameterFix(CppContainerList<CppParameter> parameters)
	{
		List<SKParameter> result = new List<SKParameter>();
		// CppAst seems to have a bug with function pointer parameters where it
		// duplicates all parameters as children of the function pointer. This
		// function detects and mitigates for that by extracting the correct
		// parameter from the sub-list.
		for (int i = 0; i<parameters.Count; i+=1)
		{
			if (parameters[i].Type is CppPointerType ptr && ptr.ElementType is CppFunctionType fnPtr)
				result.Add(ParseParameter(fnPtr.Parameters[i]));
			else
				result.Add(ParseParameter(parameters[i]));
		}
		return result;
	}

	///////////////////////////////////////////

	static SKParameter ParseParameter(CppParameter param)
	{
		SKParameter result = new();
		result.name         = param.Name;
		result.nameFlagless = "";
		result.defaultValue = param.InitExpression;
		result.type         = ParseType(param.Type, param.Name);
		result.stringType   = result.type.name == "char" ? SKStringType.ASCII : SKStringType.None;
		if (result.type.functionPtr != null)
			result.type.name = param.Name;

		string[] words = param.Name.Split('_');
		foreach (string word in words)
		{
			if      (word == "in"   ) result.passType   = SKPassType.In;
			else if (word == "out"  ) result.passType   = SKPassType.Out;
			else if (word == "ref"  ) result.passType   = SKPassType.Ref;
			else if (word == "opt"  ) result.type.isOptional = true;
			else if (word == "arr"  ) result.isArrayPtr = true;
			else if (word == "utf8" ) result.stringType = SKStringType.UTF8;
			else if (word == "utf16") result.stringType = SKStringType.UTF16;
			else if (word == "ascii") result.stringType = SKStringType.ASCII;
			else
			{
				result.nameFlagless = string.IsNullOrEmpty(result.nameFlagless)
					? word
					: result.nameFlagless + "_" + word;
			}
		}
		return result;
	}
}