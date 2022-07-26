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

///////////////////////////////////////////

struct SKType
{
	public string          raw;
	public string          final;
	public SKTypeDirection direction;
	public bool            array;
	public SKType(string raw, string final, SKTypeDirection dir = SKTypeDirection.None, bool array = false) { this.raw = raw;   this.final = final; this.direction = dir; this.array = array; }
	public SKType(string final,             SKTypeDirection dir = SKTypeDirection.None, bool array = false) { this.raw = final; this.final = final; this.direction = dir; this.array = array; }

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

	public static SKType TypeName(CppType type, string varName, Dictionary<string, string> delegateDefinitions)
	{
		string name = type.GetDisplayName();

		if (StereoKitTypes.types.Contains(name)) return new SKType("IntPtr", SnakeToCamel(name, true, 0));
		if (type.TypeKind == CppTypeKind.Qualified && ((CppQualifiedType)type).Qualifier == CppTypeQualifier.Const)
		{
			CppQualifiedType cn     = (CppQualifiedType)type;
			CppType          cnType = cn.ElementType;
			if (StereoKitTypes.types.Contains(cnType.GetDisplayName())) return new SKType("IntPtr", SnakeToCamel(cnType.GetDisplayName(), true, 0));
		}
		
		if (type.TypeKind == CppTypeKind.Pointer || type.TypeKind == CppTypeKind.Reference)
		{
			bool            array = varName.Contains("_arr_");
			SKTypeDirection dir   = SKTypeDirection.None;
			if      (varName.StartsWith("out_")) dir = SKTypeDirection.Out;
			else if (varName.StartsWith("in_" )) dir = SKTypeDirection.In;
			else if (varName.StartsWith("ref_")) dir = SKTypeDirection.Ref;

			CppType refType = type.TypeKind == CppTypeKind.Pointer
				? ((CppPointerType  )type).ElementType
				: ((CppReferenceType)type).ElementType;

			// Check for void**
			if (refType.TypeKind == CppTypeKind.Pointer || refType.TypeKind == CppTypeKind.Reference)
			{
				CppType refRefType = refType.TypeKind == CppTypeKind.Pointer
					? ((CppPointerType  )refType).ElementType
					: ((CppReferenceType)refType).ElementType;
				if (refRefType.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)refRefType).Kind == CppPrimitiveKind.Void) return new SKType("IntPtr");
			}
			// Check for void*
			if (refType.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)refType).Kind == CppPrimitiveKind.Void) return new SKType("IntPtr");
			// Check for function pointer
			if (refType.TypeKind == CppTypeKind.Function)
			{
				delegateDefinitions[CallbackType((CppFunctionType)refType)] = CallbackDefinition((CppFunctionType)refType);
				return new SKType(CallbackParameter((CppFunctionType)refType));
			}
			// Check for const *
			if (refType.TypeKind == CppTypeKind.Qualified && ((CppQualifiedType)refType).Qualifier == CppTypeQualifier.Const)
			{
				if (dir == SKTypeDirection.None && varName != "") dir = SKTypeDirection.In;
				
				CppQualifiedType cn = (CppQualifiedType)refType;
				if (cn.ElementType.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)cn.ElementType).Kind == CppPrimitiveKind.Void) return new SKType("IntPtr");
				if (cn.ElementType.TypeKind == CppTypeKind.Primitive && ((CppPrimitiveType)cn.ElementType).Kind == CppPrimitiveKind.Char) return new SKType("IntPtr", "string");

				return new SKType(SnakeToCamel(cn.ElementType.GetDisplayName(), true, 0), dir, array);
			}

			return new SKType(SnakeToCamel(refType.GetDisplayName(), true, 0), dir, array);
		}
		
		return new SKType(SnakeToCamel(name, true, 0));
	}
}
