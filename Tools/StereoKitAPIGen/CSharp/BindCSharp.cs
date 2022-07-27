using CppAst;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

class BindCSharp
{
	public static void Bind(CppCompilation ast, string outputFolder)
	{
		StereoKitTypes.Parse(ast);
		
		StringBuilder enumText = new StringBuilder();
		enumText.Append(@"// This is a generated file based on stereokit.h! Please don't modify it
// directly :) Instead, modify the header file, and run the StereoKitAPIGen
// project.

using System;

namespace StereoKit
{
");

		var ns = ast.Namespaces[0];
		foreach (var e in ns.Enums)
		{
			BuildEnum(enumText, e, 1);
			enumText.AppendLine("");
		}
		enumText.AppendLine("}");

		File.WriteAllText(Path.Combine(outputFolder, "NativeEnums.cs"), enumText.ToString());

		StringBuilder fnFileText = new StringBuilder();
		StringBuilder fnText     = new StringBuilder();
		Dictionary<string, string> delegates = new Dictionary<string, string>();
		foreach (var m in ParseModules(ast))
			m.BuildRawModule(fnText, delegates, "\t\t");

		fnFileText.Append(@"// This is a generated file based on stereokit.h! Please don't modify it
// directly :) Instead, modify the header file, and run the StereoKitAPIGen
// project.

using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	internal static partial class NativeAPI
	{
		const string            dll   = ""StereoKitC"";
		const CharSet           ascii = CharSet.Ansi;
		const CharSet           utf16 = CharSet.Unicode;
		const CallingConvention call  = CallingConvention.Cdecl;

		///////////////////////////////////////////

");

		foreach (string d in delegates.Values)
			fnFileText.AppendLine($"\t\t[UnmanagedFunctionPointer(CallingConvention.Cdecl)] {d}");
		fnFileText.Append(fnText);
		fnFileText.AppendLine("	}\r\n}");

		File.WriteAllText(Path.Combine(outputFolder, "NativeAPI.cs"), fnFileText.ToString());
		//Console.WriteLine(fnFileText.ToString());
	}

	///////////////////////////////////////////

	static string BuildExpression(CppExpression expression, string removePrefix)
	{
		string result = "";
		CppExpression exp = expression;
		while (exp != null)
		{
			CppExpression curr = exp;
			if (exp is CppBinaryExpression) curr = ((CppBinaryExpression)exp).Arguments[1];
			
			switch(curr.Kind)
			{
				case CppExpressionKind.DeclRef: {
					string name   = ((CppRawExpression)curr).Text;
					int    prefix = 0;
					if (name.StartsWith(removePrefix)) prefix = removePrefix.Length;
					result = CSTypes.SnakeToCamel(name, true, prefix) + result;
				} break;
				default: result = curr + result; break;
			}

			if (exp is CppBinaryExpression) {
				result = ((CppBinaryExpression)exp).Operator + " " + result;
				exp    = ((CppBinaryExpression)exp).Arguments[0];
				result = " " + result;
			} else {
				exp = null;
			}
		}
		return result;
	}

	///////////////////////////////////////////

	static bool IsCommand(CppComment cmd)
	{
		if (cmd.Kind == CppCommentKind.InlineCommand) return true;
		if (cmd.Kind == CppCommentKind.Text) { string txt = ((CppCommentText)cmd).Text; return txt == "@" || txt == "&" || txt == "<" || txt == ">"; }
		return false;
	}

	static string BuildCommentSummary(CppComment comment, int indent)
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


		string   prefix = new string('\t', indent) + "/// ";
		string[] lines  = txt.Split("\n");
		bool     first  = true;
		string   obsoleteStr = "";

		for (int i = 0; i < lines.Length; i++)
		{
			if (lines[i].StartsWith("obsolete:")) {
				obsoleteStr = $"\r\n{new string('\t', indent)}[Obsolete(\"{lines[i].Substring("obsolete:".Length).Trim()}\")]";
				lines[i] = "";
			}
			else {
				lines[i] = prefix + (first ? "<summary>" : "") + lines[i];
				first = false;
			}
		}
		
		lines[lines.Length - 1] = lines[lines.Length - 1] + "</summary>";
		return string.Join("\r\n", lines) + obsoleteStr;
	}

	///////////////////////////////////////////

	static void BuildEnum(StringBuilder enumText, CppEnum e, int indentPrefix)
	{
		bool flags = false;
		foreach (var i in e.Items)
		{
			if (i.ValueExpression != null && i.ValueExpression.ToString().Contains("<<"))
			{
				flags = true;
				break;
			}
		}

		string prefix = new string('\t', indentPrefix);

		if (e.Comment != null) enumText.AppendLine(BuildCommentSummary(e.Comment, indentPrefix));
		if (flags)             enumText.AppendLine($"{prefix}[Flags]");
		enumText.AppendLine($"{prefix}public enum {CSTypes.SnakeToCamel(e.Name, true, 0)} {{");
		foreach (var i in e.Items)
		{
			if (i.Comment         != null) enumText.AppendLine(BuildCommentSummary(i.Comment, indentPrefix+1));
			if (i.ValueExpression == null) enumText.AppendLine($"{prefix}	{CSTypes.SnakeToCamel(i.Name, true, e.Name.Length)},");
			else                           enumText.AppendLine($"{prefix}	{CSTypes.SnakeToCamel(i.Name, true, e.Name.Length),-12} = {BuildExpression(i.ValueExpression, e.Name)},");
		}
		enumText.AppendLine($"{prefix}}}");
	}

	///////////////////////////////////////////

	static List<CSModule> ParseModules(CppCompilation ast)
	{
		Dictionary<string, CSModule> modules = new Dictionary<string, CSModule>();
		
		foreach (var f in ast.Functions)
		{
			string[] words = f.Name.Split('_');
			CSModule mod   = null;
			if (!modules.TryGetValue(words[0], out mod))
			{
				mod = new CSModule(words[0]);
				modules.Add(words[0], mod);
			}

			mod.AddFunction(f);
		}

		return new List<CSModule>(modules.Values);
	}
}
