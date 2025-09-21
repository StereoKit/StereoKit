using System;
using System.IO;
using System.Linq;
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

			#nullable enable

			using System;
			using System.Runtime.InteropServices;
			using System.Runtime.CompilerServices;
			
			[assembly: DisableRuntimeMarshalling]

			namespace StereoKit;
			
			internal static unsafe partial class NativeAPI
			{
				const string            dll   = "StereoKitC";
				const StringMarshalling utf16 = StringMarshalling.Utf16;
				const StringMarshalling utf8  = StringMarshalling.Utf8;
				const StringMarshalling ascii = StringMarshalling.Utf8;

			""");

		foreach (SKModule m in data.modules)
			BuildModuleFunctions(data, fnFileText, m);

		foreach (var f in data.functions
			.SelectMany(f   => f.parameters)
			.Select    (p   => p.type.functionPtr)
			.Where     (ptr => ptr != null)
			.Distinct  ()) {
			BuildDelegate(data, fnFileText, f.Val);
		}

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

		string name = NameObject(e.name);
		if (name == "_") return;

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

			string paramList = "";
			for (int i = 0; i < f.parameters.Length; i++)
			{
				SKParameter p = f.parameters[i];
				paramList += $"{PInvokeParamToStr(p, data.modules)} {NameParam(p.nameFlagless)}{(i == f.parameters.Length-1 ? "" : ", ")}";
			}
			bool   isAsset       = SKHeaderData.IsAsset(f.returnType, data.modules);
			string returnTypeStr = ReturnTypeToStr(f.returnType);

			text.AppendLine($"\t// {f.src.ToString()}");

			text.Append($"\t[DllImport(dll)][UnmanagedCallConv(CallConvs = new[] {{typeof(CallConvCdecl)}})]");
			text.Append($" public static extern ");
			text.Append($"{(isAsset ? "IntPtr" : returnTypeStr + (f.returnType.pointerLvl == 1 && returnTypeStr != "string" && returnTypeStr != "IntPtr" ? "*" : "")),-15} ");
			text.AppendLine($"{f.name, -30}({paramList});");
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
		return NameType(type.name);
	}

	static string ReturnTypeToStr(SKType type)
	{
		return TypeToStr(type);
	}

	static string PInvokeParamToStr(SKParameter param, SKModule[] modules)
	{
		if (SKHeaderData.IsAsset(param.type, modules))
			return "IntPtr";

		string prefix  = "";
		string postfix = "";

		if (param.type.pointerLvl > 0)
		{
			for (int i = 0; i < param.type.pointerLvl; i++)
				postfix += "*";
		}
		else if (param.type.isOptional)
		{
			postfix += "*";
		}
		else if (param.isArrayPtr)
		{
			if (param.passType == SKPassType.Out)
				return "out IntPtr";
			if (param.passType == SKPassType.Ref)
				prefix = "[In, Out] ";
			postfix = "[]";
		}
		else
		{
			if (param.passType != SKPassType.None)
				prefix = $"{param.passType.ToString().ToLower()} ";
		}

		if (param.type.arraySize1 > 0)
		{
			prefix  = $"[MarshalAs(UnmanagedType.LPArray, SizeConst = {param.type.arraySize1})] " + prefix;
			postfix = "[]";
		}

		string name = TypeToStr(param.type);
		if (param.type.functionPtr != null)
		{
			SKFunction fn = param.type.functionPtr.Val;

			name = "delegate*<";
			for (int i = 0; i < fn.parameters.Length; i++)
				name += $"{DelegateTypeToStr(fn.parameters[i].type, modules)},";
			
			if (fn.returnType.IsVoid)
				name += "void>";
			else
				name += $"{DelegateTypeToStr(fn.returnType, modules)}>";
			
		}
		return $"{prefix}{name}{postfix}";
	}

	static string DelegateTypeToStr(SKType type, SKModule[] modules)
	{
		if (type.name == "bool32_t") return "int";
		if (type.pointerLvl > 0 && type.name != "char") return "IntPtr";
		if (SKHeaderData.IsAsset(type, modules)) return "IntPtr";
		return TypeToStr(type);
	}

	static void BuildDelegate(SKHeaderData data, StringBuilder text, SKFunction d)
	{
		//text.AppendLine($"\t[UnmanagedFunctionPointer(CallingConvention.Cdecl)] {d.name}");
	}
}