using CppAst;
using System.Collections.Generic;

///////////////////////////////////////////

enum SKTypeDirection
{
	None,
	In,
	Out,
	Ref
}
enum SKSpecialType
{
	None,
	VoidPtr,
	Ascii,
	Utf8,
	Utf16,
}

///////////////////////////////////////////

struct SKType
{
	public string          raw;
	public SKTypeDirection direction;
	public SKSpecialType   special;
	public bool            array;
	public int             fixedArray;
	public bool            constant;
	public SKType(string raw, SKTypeDirection dir = SKTypeDirection.None, bool array = false, bool constant = false, SKSpecialType special = SKSpecialType.None, int fixedArray = 0) { this.fixedArray = fixedArray; this.special = special; this.raw = raw; this.direction = dir; this.array = array; this.constant = constant; }

	public static SKType Create(CppType type, string varName, bool constant = false, int pointer = 0, int arraySize = 0)
	{
		// const
		if (type.TypeKind == CppTypeKind.Qualified && ((CppQualifiedType)type).Qualifier == CppTypeQualifier.Const)
			return Create(((CppQualifiedType)type).ElementType, varName, true, pointer, arraySize);
		
		// *
		if (type.TypeKind == CppTypeKind.Pointer)
			return Create(((CppPointerType)type).ElementType, varName, constant, pointer + 1, arraySize);

		// &
		if (type.TypeKind == CppTypeKind.Reference)
			return Create(((CppReferenceType)type).ElementType, varName, constant, pointer + 1, arraySize);

		// Fixed size arrays []
		if (type.TypeKind == CppTypeKind.Array)
			return Create(((CppArrayType)type).ElementType, varName, constant, pointer + 1, ((CppArrayType)type).Size);

		bool            array = varName.Contains("_arr_") || arraySize > 0;
		SKTypeDirection dir   = SKTypeDirection.None;
		if      (varName.StartsWith("out_")) dir = SKTypeDirection.Out;
		else if (varName.StartsWith("in_" )) dir = SKTypeDirection.In;
		else if (varName.StartsWith("ref_")) dir = SKTypeDirection.Ref;

		SKSpecialType special = SKSpecialType.None;
		if      (varName.EndsWith("_ptr"  )) special = SKSpecialType.VoidPtr;
		else if (varName.EndsWith("_utf8" )) special = SKSpecialType.Utf8;
		else if (varName.EndsWith("_utf16")) special = SKSpecialType.Utf16;
		else if (varName.EndsWith("_ascii")) special = SKSpecialType.Ascii;

		return new SKType(type.GetDisplayName(), dir, array, constant, special, arraySize);
	}
}

///////////////////////////////////////////

class Tools
{
	static bool IsCommand(CppComment cmd)
	{
		if (cmd.Kind == CppCommentKind.InlineCommand) return true;
		if (cmd.Kind == CppCommentKind.Text) { string txt = ((CppCommentText)cmd).Text; return txt == "@" || txt == "&" || txt == "<" || txt == ">"; }
		return false;
	}

	public static string BuildCommentSummary(CppComment comment)
	{
		// Assemble a comment that ignores inline commands
		string txt        = "";
		var    children   = comment.Children[0].Children;
		for (int i = 0; i < children.Count; i++)
		{
			bool isCommand   = IsCommand(children[i]);
			bool nextCommand = i+1 == children.Count
				? false
				: IsCommand(children[i+1]);

			if (children[i] is CppCommentInlineCommand) txt += "@" + ((CppCommentInlineCommand)children[i]).CommandName; 
			else                                        txt += children[i].ToString();
			
			if (!(nextCommand || isCommand) && i+1 < children.Count)
				txt += "\n";
		}
		txt = txt.Replace("&", "&amp;");
		txt = txt.Replace("<", "&lt;");
		txt = txt.Replace(">", "&gt;");

		return txt;
	}
}