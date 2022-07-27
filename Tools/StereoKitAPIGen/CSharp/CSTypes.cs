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
	public SKType(string raw, string final, SKTypeDirection dir = SKTypeDirection.None, bool array = false, SKTextType text = SKTextType.None) { this.text = text; this.raw = raw;   this.final = final; this.direction = dir; this.array = array; }
	public SKType(string final,             SKTypeDirection dir = SKTypeDirection.None, bool array = false, SKTextType text = SKTextType.None) { this.text = text; this.raw = final; this.final = final; this.direction = dir; this.array = array; }

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

	public static string CallbackType(CppFunctionType fn)
	{
		string result = fn.ReturnType.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)fn.ReturnType).Kind == CppPrimitiveKind.Void
			? "SKAction"
			: "SKFunc";

		for (int i = 0; i < fn.Parameters.Count; i++) 
		{
			var p = fn.Parameters[i];
			result += "_" + TypeName(p.Type, "", null).raw;
		}

		return result;
	}

	///////////////////////////////////////////
	
	public static string CallbackDefinition(CppFunctionType fn)
	{
		string ret = fn.ReturnType.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)fn.ReturnType).Kind == CppPrimitiveKind.Void
			? "void"
			: TypeName(fn.ReturnType, "", null).raw;

		string result = $"internal delegate {ret} {CallbackType(fn)}(";
		for (int i = 0; i < fn.Parameters.Count; i++)
		{
			if (i != 0) result += ", ";

			var p = fn.Parameters[i];
			result += TypeName(p.Type, "", null).raw + " " + p.Name;
		}
		result += ");";

		return result;
	}

	///////////////////////////////////////////

	public static string CallbackParameter(CppFunctionType fn)
	{
		return $"[MarshalAs(UnmanagedType.FunctionPtr)] {CallbackType(fn)}";
	}

	///////////////////////////////////////////

	public static SKType TypeName(CppType type, string varName, Dictionary<string, string> delegateDefinitions, bool constant, int pointer)
	{
		// const
		if (type.TypeKind == CppTypeKind.Qualified && ((CppQualifiedType)type).Qualifier == CppTypeQualifier.Const)
			return TypeName(((CppQualifiedType)type).ElementType, varName, delegateDefinitions, true, pointer);
		
		// *
		if (type.TypeKind == CppTypeKind.Pointer)
			return TypeName(((CppPointerType)type).ElementType, varName, delegateDefinitions, constant, pointer + 1);

		// &
		if (type.TypeKind == CppTypeKind.Reference)
			return TypeName(((CppReferenceType)type).ElementType, varName, delegateDefinitions, constant, pointer + 1);

		bool            array = varName.Contains("_arr_");
		SKTypeDirection dir   = SKTypeDirection.None;
		if      (varName.StartsWith("out_")) dir = SKTypeDirection.Out;
		else if (varName.StartsWith("in_" )) dir = SKTypeDirection.In;
		else if (varName.StartsWith("ref_")) dir = SKTypeDirection.Ref;

		if (pointer > 0 && constant && varName != "")
			dir = SKTypeDirection.In;

		if (type.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)type).Kind == CppPrimitiveKind.Void)
			return new SKType("IntPtr", dir, array);
		if (type.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)type).Kind == CppPrimitiveKind.Char)
		{
			return varName.EndsWith("_utf8")
				? new SKType("byte",   "string", dir, true,  SKTextType.Utf8)
				: new SKType("string", "string", dir, array, SKTextType.Ascii);
		}

		if (type.TypeKind == CppTypeKind.Function)
		{
			delegateDefinitions[CallbackType((CppFunctionType)type)] = CallbackDefinition((CppFunctionType)type);
			return new SKType(CallbackParameter((CppFunctionType)type));
		}

		string name = type.GetDisplayName();
		if (name == "const char16_t" || name == "char16_t")
			return new SKType("string", "string", dir, array, SKTextType.Utf16);

		return StereoKitTypes.types.Contains(name)
			? new SKType("IntPtr", SnakeToCamel(name, true, 0))
			: new SKType(SnakeToCamel(name, true, 0), dir, array);
	}
	public static SKType TypeName(CppType type, string varName, Dictionary<string, string> delegateDefinitions)
		=> TypeName(type, varName, delegateDefinitions, false, 0);
	
}
