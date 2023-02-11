using CppAst;
using System.Collections.Generic;
using System.Linq;

///////////////////////////////////////////

enum SKTypeDirection
{
	None,
	In,
	Out,
	Ref
}
enum SKTextType
{
	None,
	Ascii,
	Utf8,
	Utf16,
}

///////////////////////////////////////////

struct SKType
{
	public string          raw;
	public string          final;
	public SKTypeDirection direction;
	public SKTextType      text;
	public bool            array;
	public int             fixedArray;
	public SKType(string raw, string final, SKTypeDirection dir = SKTypeDirection.None, bool array = false, SKTextType text = SKTextType.None, int fixedArray = 0) { this.fixedArray = fixedArray; this.text = text; this.raw = raw;   this.final = final; this.direction = dir; this.array = array; }
	public SKType(string final,             SKTypeDirection dir = SKTypeDirection.None, bool array = false, SKTextType text = SKTextType.None, int fixedArray = 0) { this.fixedArray = fixedArray; this.text = text; this.raw = final; this.final = final; this.direction = dir; this.array = array; }

	public string RawName => array
		? $"{(direction != SKTypeDirection.None ? $"[{(direction == SKTypeDirection.Ref ? "In, Out" : direction)}] " : "")}{raw}[]"
		: $"{(direction != SKTypeDirection.None ? direction.ToString().ToLower() + " " : "")}{raw}";
	public string FinalName => array
		? $"{(direction != SKTypeDirection.None ? $"[{(direction == SKTypeDirection.Ref ? "In, Out" : direction)}] " : "")}{final}[]"
		: $"{(direction != SKTypeDirection.None ? direction.ToString().ToLower() + " " : "")}{final}";
}

///////////////////////////////////////////

class CSTypes
{
	public static string SnakeToCamel(string name, bool active, int removePrefix)
	{
		if (NameOverrides.TryGet(name, out string result))
			return result;

		string[] words = name.Substring(removePrefix).Split("_");
		if (words.Length > 1 && words[words.Length - 1] == "t") words[words.Length - 1] = "";
		for (int i = 0; i < words.Length; i++)
		{
			if ((i > 0 || active) && words[i].Length>0)
				words[i] = char.ToUpper( words[i][0]) + words[i].Substring(1);
		}
		return string.Join("", words);
	}

	///////////////////////////////////////////

	public static string CallbackType(CppFunctionType fn, string varName)
	{
		bool isVoidRet = fn.ReturnType.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)fn.ReturnType).Kind == CppPrimitiveKind.Void;
		if (isVoidRet == true && fn.Parameters.Count == 0)
			return "Action";

		string result = SnakeToCamel(varName, true, 0);

		/*for (int i = 0; i < fn.Parameters.Count; i++) 
		{
			var p = fn.Parameters[i];
			result += "_" + TypeName(p.Type, p.Name, null).raw;
		}*/
		
		return result;
	}

	///////////////////////////////////////////
	
	public static string CallbackDefinition(CppFunctionType fn, string varName)
	{
		bool isVoidRet = fn.ReturnType.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)fn.ReturnType).Kind == CppPrimitiveKind.Void;
		if (isVoidRet == true && fn.Parameters.Count == 0)
			return "";

		string ret = isVoidRet
			? "void"
			: TypeName(fn.ReturnType, "", null).raw;

		string result = $"internal delegate {ret} {CallbackType(fn, varName)}(";
		for (int i = 0; i < fn.Parameters.Count; i++)
		{
			if (i != 0) result += ", ";

			var p = fn.Parameters[i];
			result += TypeName(p.Type, p.Name, null).RawName + " " + SnakeToCamel(p.Name, false, 0);
		}
		result += ");";

		return result;
	}

	///////////////////////////////////////////

	public static string CallbackParameter(CppFunctionType fn, string varName)
	{
		return $"[MarshalAs(UnmanagedType.FunctionPtr)] {CallbackType(fn, varName)}";
	}

	///////////////////////////////////////////

	public static SKType TypeName(CppType type, string varName, Dictionary<string, string> delegateDefinitions, bool constant, int pointer, int arraySize)
	{
		// const
		if (type.TypeKind == CppTypeKind.Qualified && ((CppQualifiedType)type).Qualifier == CppTypeQualifier.Const)
			return TypeName(((CppQualifiedType)type).ElementType, varName, delegateDefinitions, true, pointer, arraySize);
		
		// *
		if (type.TypeKind == CppTypeKind.Pointer)
			return TypeName(((CppPointerType)type).ElementType, varName, delegateDefinitions, constant, pointer + 1, arraySize);

		// &
		if (type.TypeKind == CppTypeKind.Reference)
			return TypeName(((CppReferenceType)type).ElementType, varName, delegateDefinitions, constant, pointer + 1, arraySize);

		// Fixed size arrays []
		if (type.TypeKind == CppTypeKind.Array)
			return TypeName(((CppArrayType)type).ElementType, varName, delegateDefinitions, constant, pointer + 1, ((CppArrayType)type).Size);

		bool            array = varName.Contains("_arr_") || arraySize > 0;
		SKTypeDirection dir   = SKTypeDirection.None;
		if      (varName.StartsWith("out_")) dir = SKTypeDirection.Out;
		else if (varName.StartsWith("in_" )) dir = SKTypeDirection.In;
		else if (varName.StartsWith("ref_")) dir = SKTypeDirection.Ref;

		string name = type.GetDisplayName();

		// Allow for an IntPtr override
		if (varName.EndsWith("_ptr"))
			return new SKType("IntPtr", SnakeToCamel(name, true, 0), dir, array, SKTextType.None, arraySize);
		
		if (dir == SKTypeDirection.None && pointer > 0 && constant && varName != "")
			dir = SKTypeDirection.In;

		if (pointer > 0 && type.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)type).Kind == CppPrimitiveKind.Void)
		{
			return varName == "data"
				? new SKType("byte", dir, true, SKTextType.None, arraySize)
				: new SKType("IntPtr", dir, array, SKTextType.None, arraySize);
		}
		if (type.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)type).Kind == CppPrimitiveKind.Char)
		{
			if (varName.EndsWith("_utf8"))
			{
				return new SKType("byte", "string", dir, true, SKTextType.Utf8, arraySize);
			}
			else
			{
				return dir == SKTypeDirection.Out || varName == ""
					? new SKType("IntPtr", "string", dir, array, SKTextType.Ascii, arraySize)
					: new SKType("string", "string", array ? dir : SKTypeDirection.None, array, SKTextType.Ascii, arraySize);
			}
		}

		if (type.TypeKind == CppTypeKind.Function)
		{
			delegateDefinitions[CallbackType((CppFunctionType)type, varName)] = CallbackDefinition((CppFunctionType)type, varName);
			return new SKType(CallbackParameter((CppFunctionType)type, varName));
		}
		
		if (name == "const char16_t" || name == "char16_t")
			return new SKType("string", "string", dir == SKTypeDirection.In ? SKTypeDirection.None : dir, array, SKTextType.Utf16, arraySize);

		// pointer return types
		if (varName == "" && pointer > 0)
		{
			return new SKType("IntPtr", SnakeToCamel(name, true, 0), dir, array, SKTextType.None, arraySize);
		}
		
		return StereoKitTypes.types.Contains(name)
			? new SKType("IntPtr", SnakeToCamel(name, true, 0))
			: new SKType(SnakeToCamel(name, true, 0), dir, array, SKTextType.None, arraySize);
	}
	public static SKType TypeName(CppType type, string varName, Dictionary<string, string> delegateDefinitions)
		=> TypeName(type, varName, delegateDefinitions, false, 0, 0);
	
}
