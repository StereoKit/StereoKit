using CppAst;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class BindZig
{
	public static void Bind(CppCompilation ast, string outputFolder)
	{
		StereoKitTypes.Parse(ast);
		
		StringBuilder text = new StringBuilder();
		text.Append(@"// This is a generated file based on stereokit.h! Please don't modify it
// directly :) Instead, modify the header file, and run the StereoKitAPIGen
// project.

");

		var ns = ast.Namespaces[0];
		foreach (var e in ns.Enums)
		{
			BuildEnum(text, e, 0);
			text.AppendLine("");
		}

		Dictionary<string, string> delegates = new Dictionary<string, string>();
		foreach (var m in ParseModules(ast))
			m.BuildRawModule(text, delegates, "");

		//foreach (string d in delegates.Values)
		//	if (!string.IsNullOrEmpty(d)) text.AppendLine($"\t\t[UnmanagedFunctionPointer(CallingConvention.Cdecl)] {d}");

		File.WriteAllText(Path.Combine(outputFolder, "StereoKit.zig"), text.ToString());
		Console.WriteLine(text.ToString());
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
		//if (flags)             enumText.AppendLine($"{prefix}[Flags]");
		enumText.AppendLine($"{prefix}pub const {CSTypes.SnakeToCamel(e.Name, true, 0)} = enum(c_int) {{");
		foreach (var i in e.Items)
		{
			if (i.Comment         != null) enumText.AppendLine(BuildCommentSummary(i.Comment, indentPrefix+1));
			if (i.ValueExpression == null) enumText.AppendLine($"{prefix}	{CSTypes.SnakeToCamel(i.Name, true, e.Name.Length)},");
			else                           enumText.AppendLine($"{prefix}	{CSTypes.SnakeToCamel(i.Name, true, e.Name.Length),-12} = {BindCSharp.BuildExpression(i.ValueExpression, e.Name)},");
		}
		enumText.AppendLine($"{prefix}}};");
	}

	///////////////////////////////////////////

	static List<ZigModule> ParseModules(CppCompilation ast)
	{
		Dictionary<string, ZigModule> modules = new Dictionary<string, ZigModule>();

		foreach (var c in ast.Namespaces[0].Classes)
		{
			string modName = CSTypes.SnakeToCamel(c.Name.EndsWith("_t")
				? c.Name.Substring(0,c.Name.Length-2)
				: c.Name, true, 0);

			ZigModule mod = null;
			if (!modules.TryGetValue(modName, out mod))
			{
				mod = new ZigModule(modName);
				modules.Add(modName, mod);
			}

			if (c.Name.StartsWith("_"))
			{
				mod.isAsset = true;
				continue;
			}

			foreach (var f in c.Fields)
				mod.AddField(f);
		}

		foreach (var f in ast.Functions)
		{
			string[]  words = f.Name.Split('_');
			ZigModule mod   = null;
			string modName = CSTypes.SnakeToCamel(words[0], true, 0);
			if (!modules.TryGetValue(modName, out mod))
			{
				mod = new ZigModule(modName);
				modules.Add(modName, mod);
			}

			mod.AddFunction(f);
		}

		return new List<ZigModule>(modules.Values);
	}

	///////////////////////////////////////////

	public static string BuildCommentSummary(CppComment comment, int indent)
	{
		string   txt    = Tools.BuildCommentSummary(comment);
		string   prefix = new string('\t', indent) + "// ";
		string[] lines  = txt.Split("\n");

		for (int i = 0; i < lines.Length; i++)
			lines[i] = prefix + lines[i];
		
		return string.Join("\r\n", lines);
	}

	///////////////////////////////////////////
	
	public static string TypeToName(SKType type)
	{
		if      (type.special == SKSpecialType.VoidPtr) return "?*anyopaque";
		else if (type.special == SKSpecialType.Utf8   ) return type.constant ? "[*]const u8" : "[*]u8";
		else if (type.special == SKSpecialType.Ascii  ) return type.constant ? "[*]const u8" : "[*]u8";
		else return type.array
			? $"[*]{(type.constant?"const ":"")}{CSTypes.SnakeToCamel(type.raw, true, 0)}"
			: (type.constant?"const ":"")+CSTypes.SnakeToCamel(type.raw, true, 0);
	}
}