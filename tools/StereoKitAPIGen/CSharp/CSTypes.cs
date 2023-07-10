using CppAst;
using System.Collections.Generic;
using System.Linq;

///////////////////////////////////////////

class CSTypes
{
	public static string SnakeToCamel(string name, bool active, int removePrefix)
	{
		if (NameOverrides.TryGet(name, out string result))
			return result;

		string[] words = name.Substring(removePrefix).Split("_");
		if (words.Length > 1 && words[words.Length - 1] == "t") words[words.Length - 1] = "";

		bool first = true;
		for (int i = 0; i < words.Length; i++)
		{
			if (((first && active) || !first) && words[i].Length>0)
				words[i] = char.ToUpper( words[i][0]) + words[i].Substring(1);
			if (words[i] != "") first = false;
		}
		return string.Join("", words);
	}

	///////////////////////////////////////////

	public static string SnakeToCamelFn(string name, bool active, int removePrefix)
	{
		if (NameOverrides.TryGet(name, out string result))
			return result;

		string[] words = name.Substring(removePrefix).Split("_");
		bool first = true;
		for (int i = 0; i < words.Length; i++)
		{
			if (((first && active) || !first) && words[i].Length>0)
				words[i] = char.ToUpper( words[i][0]) + words[i].Substring(1);
			if (words[i] != "") first = false;
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
	
	/*public static string CallbackDefinition(CppFunctionType fn, string varName)
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
			result += BindCSharp.TypeToName(TypeName(p.Type, p.Name, null)) + " " + SnakeToCamel(p.Name, false, 0);
		}
		result += ");";

		return result;
	}

	///////////////////////////////////////////

	public static string CallbackParameter(CppFunctionType fn, string varName)
	{
		return $"[MarshalAs(UnmanagedType.FunctionPtr)] {CallbackType(fn, varName)}";
	}*/

	///////////////////////////////////////////

	/*public static SKType TypeName(CppType type, string varName, Dictionary<string, string> delegateDefinitions, bool constant, int pointer, int arraySize)
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
			return new SKType(type, "IntPtr", dir, array, constant, SKSpecialType.None, arraySize);
		
		if (dir == SKTypeDirection.None && pointer > 0 && constant && varName != "")
			dir = SKTypeDirection.In;

		if (pointer > 0 && type.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)type).Kind == CppPrimitiveKind.Void)
		{
			return varName == "data"
				? new SKType(type, "byte", dir, true, constant, SKSpecialType.None, arraySize)
				: new SKType(type, "IntPtr", dir, array, constant, SKSpecialType.None, arraySize);
		}
		if (type.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)type).Kind == CppPrimitiveKind.Char)
		{
			if (varName.EndsWith("_utf8"))
			{
				return new SKType(type, "byte", dir, true, constant, SKSpecialType.Utf8, arraySize);
			}
			else
			{
				return dir == SKTypeDirection.Out || varName == ""
					? new SKType(type, "IntPtr", dir, array, constant, SKSpecialType.Ascii, arraySize)
					: new SKType(type, "string", array ? dir : SKTypeDirection.None, array, constant, SKSpecialType.Ascii, arraySize);
			}
		}

		if (type.TypeKind == CppTypeKind.Function)
		{
			delegateDefinitions[CallbackType((CppFunctionType)type, varName)] = CallbackDefinition((CppFunctionType)type, varName);
			return new SKType(type, CallbackParameter((CppFunctionType)type, varName));
		}
		
		if (name == "const char16_t" || name == "char16_t")
			return new SKType(type, "string", dir == SKTypeDirection.In ? SKTypeDirection.None : dir, array, constant, SKSpecialType.Utf16, arraySize);

		// pointer return types
		if (varName == "" && pointer > 0)
		{
			return new SKType(type, "IntPtr", dir, array, constant, SKSpecialType.None, arraySize);
		}
		
		return StereoKitTypes.types.Contains(name)
			? new SKType(type, "IntPtr")
			: new SKType(type, SnakeToCamel(name, true, 0), dir, array, constant, SKSpecialType.None, arraySize);
	}
	public static SKType TypeName(CppType type, string varName, Dictionary<string, string> delegateDefinitions)
		=> TypeName(type, varName, delegateDefinitions, false, 0, 0);*/
	
}
