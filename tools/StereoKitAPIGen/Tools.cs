using CppAst;
using System;
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
enum SKSpecialType
{
	None,
	VoidPtr,
	Ascii,
	Utf8,
	Utf16,
	FnPtr,
}

///////////////////////////////////////////

/*struct SKType
{
	public string          raw;
	public SKTypeDirection direction;
	public SKSpecialType   special;
	public bool            array;
	public int             fixedArray;
	public bool            constant;
	public int             pointer;
	public CppType         source;
	public SKType(CppType source, string raw, SKTypeDirection dir = SKTypeDirection.None, bool array = false, bool constant = false, SKSpecialType special = SKSpecialType.None, int fixedArray = 0, int pointer = 0) { this.source = source; this.fixedArray = fixedArray; this.special = special; this.raw = raw; this.direction = dir; this.array = array; this.constant = constant; this.pointer = pointer; }

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
		else if (type is CppPrimitiveType && ((CppPrimitiveType)type).Kind == CppPrimitiveKind.Char) special = SKSpecialType.Utf8;

		if (type.TypeKind == CppTypeKind.Function)
		{
			special = SKSpecialType.FnPtr;
		}

		return new SKType(type, type.GetDisplayName(), dir, array, constant, special, arraySize, pointer);
	}
}*/

///////////////////////////////////////////

class Tools
{
	public static string ToStrComment(string comment, string prefix, int columns = 80)
	{
		int    tabExtra  = prefix.Count(c => c == '\t') * 3;
		string lineStart = $"{prefix}";
		string result = "";
		string[] paragraphs = comment.Split("\n");
		for (int i = 0; i < paragraphs.Length; i++)
		{
			string[] words = paragraphs[i].Split(" ");
			string line = lineStart;
			for (int w = 0; w < words.Length; w++)
			{
				if (line.Length + words[w].Length + 2 + tabExtra > columns)
				{
					result += line;
					line = "\r\n" + lineStart;
				}
				line += " " + words[w];
			}
			result += line;
			if (i < paragraphs.Length - 1)
				result += $"\r\n{prefix}\r\n";
		}
		return result;
	}

	///////////////////////////////////////////

	public static string BuildExpression(CppExpression exp, Func<string, string> renamer)
	{
		string result = "";
		while (exp != null)
		{
			CppExpression curr = exp;
			if (exp is CppBinaryExpression bExp) curr = bExp.Arguments[1];
			
			switch(curr.Kind)
			{
				case CppExpressionKind.DeclRef: {
					result = renamer(((CppRawExpression)curr).Text) + result;
				} break;
				case CppExpressionKind.UnaryOperator: {
					CppExpression arg  = ((CppUnaryExpression)curr).Arguments[0];
					string        name = arg.ToString();
					if (arg.Kind == CppExpressionKind.DeclRef) {
						name = renamer(name);
					}
					result = ((CppUnaryExpression)curr).Operator + name + result;
				} break;
				default: result = curr + result; break;
			}

			if (exp is CppBinaryExpression bExp2) {
				result = " " + bExp2.Operator + " " + result;
				exp    = bExp2.Arguments[0];
			} else {
				exp = null;
			}
		}
		return result;
	}

	///////////////////////////////////////////

	public static long EvaluateIntExpression(CppExpression exp, Func<string, long> evalName)
	{
		if (exp is CppBinaryExpression bExp)
		{
			long a = EvaluateIntExpression(bExp.Arguments[0], evalName);
			long b = EvaluateIntExpression(bExp.Arguments[1], evalName);

			if      (bExp.Operator == "<<") return a << (int)b;
			else if (bExp.Operator == ">>") return a >> (int)b;
			else if (bExp.Operator == "|") return a | b;
			else if (bExp.Operator == "&") return a & b;
			else if (bExp.Operator == "+") return a + b;
			else if (bExp.Operator == "-") return a - b;
			else throw new Exception("Unimplemented binary operator: " + bExp.Operator);
		}
		else if (exp is CppUnaryExpression uExp)
		{
			long a = EvaluateIntExpression(uExp.Arguments[0], evalName);

			if      (uExp.Operator == "-") return -a;
			else if (uExp.Operator == "~") return ~a;
			else throw new Exception("Unimplemented unary operator: " + uExp.Operator);
		}
		else if (exp.Kind == CppExpressionKind.DeclRef)
		{
			return evalName(((CppRawExpression)exp).Text);
		}
		else if (exp.Kind == CppExpressionKind.IntegerLiteral)
		{
			string val = ((CppLiteralExpression)exp).Value;

			if (val.StartsWith("0x")) return Convert.ToInt32(val, 16);
			else return long.Parse(val);
		}
		else
		{
			throw new Exception("Unimplemented expression type: " + exp.Kind);
		}
	}

	///////////////////////////////////////////

	public static string SnakeToCamelU(string snake)
	{
		string[] words = snake.Split("_");

		for (int i = 0; i < words.Length; i++)
			words[i] = words[i].Length > 0 ? char.ToUpper(words[i][0]) + words[i].Substring(1) : "";
		return string.Join("", words);
	}

	///////////////////////////////////////////

	public static string SnakeToCamelL(string snake)
	{
		string[] words = snake.Split("_");

		bool first = true;
		for (int i = 0; i < words.Length; i++)
		{
			if (!first && words[i].Length > 0)
				words[i] = char.ToUpper(words[i][0]) + words[i].Substring(1);
			if (words[i] != "") first = false;
		}
		return string.Join("", words);
	}

	///////////////////////////////////////////

	public static string RemovePrefix(string str, string prefix)
		=> str.StartsWith(prefix) ? str.Substring(prefix.Length) : str;
}