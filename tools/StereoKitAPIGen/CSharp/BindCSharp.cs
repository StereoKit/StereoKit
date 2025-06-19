using CppAst;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Security.Cryptography;
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
			
			namespace StereoKit;
			

			"""
			);

		foreach (var e in data.enums)
		{
			BuildEnum(enumText, e);
			enumText.AppendLine();
		}

		File.WriteAllText(Path.Combine(outputFolder, "NativeEnums.cs"), enumText.ToString());


		StringBuilder fnFileText = new StringBuilder();
		fnFileText.Append(
			"""
			// This is a generated file based on stereokit.h! Please don't modify it
			// directly :) Instead, modify the header file, and run the StereoKitAPIGen
			// project.

			using System;
			using System.Runtime.InteropServices;
			using System.Runtime.CompilerServices;
			
			[assembly: DisableRuntimeMarshalling]

			namespace StereoKit;
			
			internal static partial class NativeAPI
			{
				const string            dll   = "StereoKitC";
				const StringMarshalling utf16 = StringMarshalling.Utf16;
				const StringMarshalling utf8  = StringMarshalling.Utf8;
				const StringMarshalling ascii = StringMarshalling.Utf8;

			""");

		foreach (var m in data.modules)
			BuildModuleFunctions(data, fnFileText, m);

		//foreach (string d in delegates.Values)
		//	if (!string.IsNullOrEmpty(d)) fnFileText.AppendLine($"\t\t[UnmanagedFunctionPointer(CallingConvention.Cdecl)] {d}");

		fnFileText.AppendLine("}\r\n");

		File.WriteAllText(Path.Combine(outputFolder, "NativeAPI2.cs"), fnFileText.ToString());
		Console.WriteLine(fnFileText.ToString());
	}

	///////////////////////////////////////////

	static string NameType(string sourceName)
		=> NameOverrides.TryGet(sourceName, out string custom)
			? custom
			: Tools.SnakeToCamelU(sourceName.EndsWith("_t") ? sourceName[..^2] : sourceName);

	static string NameObject(string sourceName)
		=> NameOverrides.TryGet(sourceName, out string custom) ? custom : Tools.SnakeToCamelU(sourceName);

	static string NameParam(string sourceName)
		=> NameOverrides.TryGet(sourceName, out string custom) ? custom : Tools.SnakeToCamelL(sourceName);

	///////////////////////////////////////////

	static string CommentText(string src)
		=> "<summary>" + src.Replace("&", "&amp;").Replace("<", "&lt;").Replace(">", "&gt;") + "</summary>";

	///////////////////////////////////////////
	
	static void BuildEnum(StringBuilder builder, SKEnum e)
	{
		Func<string, string> enumItemName = (str) =>
			NameOverrides.TryGet(str, out string custom)
				? custom
				: Tools.SnakeToCamelU(Tools.RemovePrefix(str, e.name));

		builder.AppendLine(Tools.ToStrComment(CommentText(e.comment), "///"));
		if (e.isBitflag) builder.AppendLine("[Flags]");
		builder.AppendLine($"public enum {NameObject(e.name)}\r\n{{");
		foreach (var i in e.items)
		{
			builder.AppendLine(Tools.ToStrComment(CommentText(i.comment), "\t///"));
			builder.AppendLine(i.value == null ? $"\t{enumItemName(i.name)}," : $"\t{enumItemName(i.name)} = {Tools.BuildExpression(i.value, enumItemName)},");
		}
		builder.AppendLine("}");
	}

	///////////////////////////////////////////

	static void BuildModuleFunctions(SKHeaderData data, StringBuilder text, SKModule m)
	{
		SKFunction[] moduleFunctions = SKHeaderData.GetModuleFunctions(m, data.functions);
		if (moduleFunctions.Length == 0) return;

		text.Append($"\r\n\t///////////////////////////////////////////\r\n\r\n");
		foreach (var f in moduleFunctions)
		{
			if (NameOverrides.TryGet(f.name, out string overrideName) && overrideName == "_")
				continue;

			SKStringType strType   = SKStringType.None;
			string       paramList = "";
			for (int i = 0; i < f.parameters.Length; i++)
			{
				SKParameter p = f.parameters[i];
				paramList += $"{(SKHeaderData.IsAsset(p.type, data.modules) ? "IntPtr" : PInvokeParamToStr(p))} {NameParam(p.nameFlagless)}{(i == f.parameters.Length-1 ? "" : ", ")}";

				if (p.stringType != SKStringType.None)
					strType = p.stringType;
			}
			if (strType == SKStringType.None)
				strType = f.returnType.name switch { "char" => SKStringType.ASCII, "char16_t" => SKStringType.UTF16, _ => SKStringType.None };

			if (f.returnType.name == "bool32_t")
				text.AppendLine("\t[return: MarshalAs(UnmanagedType.Bool)]");
			text.Append(strType != SKStringType.None
				? $"\t[LibraryImport(dll, StringMarshalling = {strType.ToString().ToLower(),-5})][UnmanagedCallConv(CallConvs = new[] {{typeof(CallConvCdecl)}})]"
				: $"\t[LibraryImport(dll                           )][UnmanagedCallConv(CallConvs = new[] {{typeof(CallConvCdecl)}})]");
			text.AppendLine($"public static partial {(SKHeaderData.IsAsset(f.returnType, data.modules) ? "IntPtr" : ReturnTypeToStr(f.returnType)),-15} {f.name, -30}({paramList});");
		}
	}

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

	static string TypeToStr(SKType type)
	{
		if (type.name == "char8_t" && type.pointerLvl >= 1)
			return "string";
		if ((type.name == "char" || type.name=="char16_t" ) && type.pointerLvl >= 1)
			return "string";
		if (type.name == "void" && type.pointerLvl > 0)
			return "IntPtr";
		return NameType(type.name);
	}

	static string ReturnTypeToStr(SKType type)
	{
		// Strings in the return type always need special handling
		if ((type.name == "char8_t" || type.name == "char" || type.name == "char16_t" ) && type.pointerLvl >= 1)
			return "string";
		return TypeToStr(type);
	}

	static string PInvokeParamToStr(SKParameter param)
	{
		string prefix  = "";
		string postfix = "";

		if (param.isArrayPtr)
		{
			if (param.passType == SKPassType.Out)
				return "out IntPtr";
			if (param.passType == SKPassType.Ref)
				prefix = "ref ";
			postfix = "[]";
		} else {
			if (param.passType != SKPassType.None)
				prefix = $"{param.passType.ToString().ToLower()} ";
		}

		if (param.type.name == "bool32_t")
			prefix = "[MarshalAs(UnmanagedType.Bool)] " + prefix;

		return $"{prefix}{TypeToStr(param.type)}{postfix}";
	}
}