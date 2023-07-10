using CppAst;
using StereoKitAPIGen;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

class BindCSharp
{
	public static void Bind(SKHeaderData data, string outputFolder)
	{
		StringBuilder enumText = new StringBuilder();
		enumText.Append(
			"""
			// This is a generated file based on stereokit.h! Please don't modify it
			// directly :) Instead, modify the header file, and run the StereoKitAPIGen
			// project.
			
			using System;
			
			namespace StereoKit
			{
			"""
			);

		foreach (var e in data.enums)
		{
			BuildEnum(enumText, e);
			enumText.AppendLine();
		}

		enumText.AppendLine("}");

		File.WriteAllText(Path.Combine(outputFolder, "NativeEnums.cs"), enumText.ToString());

		/*StringBuilder fnFileText = new StringBuilder();
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
			if (!string.IsNullOrEmpty(d)) fnFileText.AppendLine($"\t\t[UnmanagedFunctionPointer(CallingConvention.Cdecl)] {d}");
		fnFileText.Append(fnText);
		fnFileText.AppendLine("	}\r\n}");*/

		//File.WriteAllText(Path.Combine(outputFolder, "NativeAPI.cs"), fnFileText.ToString());
		//Console.WriteLine(fnFileText.ToString());
	}

	///////////////////////////////////////////

	static string CSObjectName(string sourceName)
		=> NameOverrides.TryGet(sourceName, out string custom) ? custom : Tools.SnakeToCamelU(sourceName);

	///////////////////////////////////////////
	
	static string CSCommentText(string src)
		=> "<summary>" + src.Replace("&", "&amp;").Replace("<", "&lt;").Replace(">", "&gt;") + "</summary>";

	///////////////////////////////////////////
	
	static void BuildEnum(StringBuilder builder, SKEnum e)
	{
		Func<string, string> enumItemName = (str) =>
			NameOverrides.TryGet(str, out string custom)
				? custom
				: Tools.SnakeToCamelU(Tools.RemovePrefix(str, e.name));

		builder.AppendLine(Tools.ToStrComment(CSCommentText(e.comment), "\t///"));
		if (e.isBitflag) builder.AppendLine("\t[Flags]");
		builder.AppendLine($"\tpublic enum {CSObjectName(e.name)}\r\n\t{{");
		foreach (var i in e.items)
		{
			builder.AppendLine(Tools.ToStrComment(CSCommentText(i.comment), "\t\t///"));
			builder.AppendLine(i.value == null ? $"\t\t{enumItemName(i.name)}," : $"\t\t{enumItemName(i.name)} = {Tools.BuildExpression(i.value, enumItemName)},");
		}
		builder.AppendLine("\t}");
	}

	///////////////////////////////////////////

	/*static List<CSModule> ParseModules(CppCompilation ast)
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
	}*/

	/*public static string TypeToName(SKType type)
	{
		return type.array
			? $"{(type.direction != SKTypeDirection.None ? $"[{(type.direction == SKTypeDirection.Ref ? "In, Out" : type.direction)}] " : "")}{type.raw}[]"
			: $"{(type.direction != SKTypeDirection.None ? type.direction.ToString().ToLower() + " " : "")}{type.raw}";
	}*/
}
