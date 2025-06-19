// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using System;
using System.IO;
using System.Linq;
using System.Text;

class BindZig
{
	public static void Bind(SKHeaderData data, string outputFolder)
	{
		StringBuilder text = new StringBuilder();
		text.Append(
			"""
			// This is a generated file based on stereokit.h! Please don't modify it
			// directly :) Instead, modify the header file, and run the StereoKitAPIGen
			// project.

			pub const bool32 = i32;
			pub const false32 = 0;
			pub const true32  = 1;

			pub const HandSimId = i32;


			""");

		foreach (var e in data.enums)
		{
			BuildEnum(text, e);
			text.AppendLine();
		}

		foreach (var m in data.modules)
			if (m.fields.Length != 0)
				BuildModule(data, text, m);

		foreach (var m in data.modules)
			if (m.fields.Length == 0 && m.isAsset)
				BuildModule(data, text, m);

		foreach (var m in data.modules)
			if (m.fields.Length == 0 && !m.isAsset)
				BuildModule(data, text, m);

		text.Append(File.ReadAllText("Zig/ZigPostfix.zig"));

		if (!Directory.Exists(outputFolder))
			Directory.CreateDirectory(outputFolder);
		File.WriteAllText(Path.Combine(outputFolder, "StereoKit.zig"), text.ToString());
		Console.WriteLine(text.ToString());
	}

	///////////////////////////////////////////

	static string NameType(string sourceName)
		=> NameOverrides.TryGet(sourceName, out string custom)
			? custom
			: Tools.SnakeToCamelU(sourceName.EndsWith("_t") ? sourceName[..^2] : sourceName);

	static string NameMethod(string sourceName, string modulePrefix)
		=> NameOverrides.TryGet(sourceName, out string custom)
			? custom
			: Tools.SnakeToCamelL(sourceName.StartsWith(modulePrefix) ? sourceName[modulePrefix.Length..] : sourceName);

	static string NameConstant(string sourceName, string modulePrefix)
		=> NameOverrides.TryGet(sourceName, out string custom)
			? custom
			: Tools.SnakeToCamelL(sourceName.StartsWith(modulePrefix) ? sourceName[modulePrefix.Length..] : sourceName);

	///////////////////////////////////////////

	static string NameField(string sourceName)
		=> NameOverrides.TryGet(sourceName, out string custom)
			? custom
			: Tools.SnakeToCamelL(sourceName);

	static string NameParam(string sourceName)
		=> NameField(sourceName);

	static string NameEnumField(string sourceName, string sourcePrefix) 
		=> NameOverrides.TryGet(sourceName, out string custom)
			? custom
			: Tools.SnakeToCamelU(Tools.RemovePrefix(sourceName, sourcePrefix));

	///////////////////////////////////////////

	static void BuildEnum(StringBuilder text, SKEnum e)
	{
		string enumName = NameType(e.name);

		Func<string, string> enumItemName = (str) => NameEnumField(str, e.name);

		Func<string, long> innerItemVal = (str) => {
			SKEnumItem item = e.items.First((i) => i.name == str);
			return Tools.EvaluateIntExpression(item.value, null);
		};

		text.AppendLine(Tools.ToStrComment(e.comment, "//"));
		text.AppendLine($"pub const {enumName} = enum(c_int) {{");
		foreach (var i in e.items)
		{
			text.AppendLine(Tools.ToStrComment(i.comment, "\t//"));
			text.AppendLine(i.value == null 
				? $"\t{enumItemName(i.name)},"
				: $"\t{enumItemName(i.name)} = {Tools.EvaluateIntExpression(i.value, innerItemVal)}, // {Tools.BuildExpression(i.value, enumItemName)}");
		}
		text.AppendLine($"}};");
	}

	///////////////////////////////////////////

	static void BuildModule(SKHeaderData data, StringBuilder text, SKModule m)
	{
		SKFunction[] moduleFunctions = SKHeaderData.GetModuleFunctions(m, data.functions);

		text.AppendLine($"\r\n///////////////////////////////////////////\r\n");

		string indent= m.name == "sk" ? "" : "\t";
		if (m.name != "sk")
			text.AppendLine($"pub const {NameType(m.name)} = extern {(m.type == SKModuleType.Union ? "union" : "struct")} {{ // {m.name}");

		// Instance value for assets
		if (m.isAsset)
			text.AppendLine($"{indent}_inst: ?*opaque{{}} = null,\r\n");

		// Fields for structs
		foreach (var f in m.fields)
		{
			string defaultVal = " = .{}";
			if (f.type.pointerLvl > 0) defaultVal = " = null";
			else if (
				f.type.name == "int64_t" ||
				f.type.name == "int32_t" ||
				f.type.name == "int16_t" ||
				f.type.name == "int8_t" ||
				f.type.name == "int8_t" ||
				f.type.name == "uint64_t" ||
				f.type.name == "uint32_t" ||
				f.type.name == "uint16_t" ||
				f.type.name == "uint8_t" ||
				f.type.name == "uint8_t" ||
				f.type.name == "float" ||
				f.type.name == "double"
				) defaultVal = " = 0";
			else if (f.type.name == "bool32_t") defaultVal = " = false32";
			else
			{
				SKEnum e = data.enums.FirstOrDefault((e) => e.name == f.type.name);
				if (e.name != null)
				{
					string enumName = NameType(e.name);
					string itemName = NameEnumField(e.items[0].name, e.name);
					defaultVal = $" = {enumName}.{itemName}";
				}
			}
			if (m.type == SKModuleType.Union) defaultVal = "";
			text.AppendLine($"{indent}{NameField(f.name) + ":"} {TypeToStr(f.type)}{defaultVal},");
		}
		if (m.fields.Length > 0) text.AppendLine();

		// Constants for the module
		SKConstant[] moduleConstants = SKHeaderData.GetModuleConstants(m, data.constants);
		foreach (var c in moduleConstants)
		{
			//string val = c.value.ToString().Replace("{", ".{");// "ahh";// Tools.EvaluateIntExpression(c.value, null).ToString();
			//text.AppendLine($"{indent}pub const {NameConstant(c.name, m.modulePrefix)}: {TypeToStr(c.type)} = {val};");
		}
		//if (moduleConstants.Length > 0) text.AppendLine();

		// These are the directly imported functions
		foreach (var f in moduleFunctions)
		{
			string paramList = f.parameters.Aggregate("", (s, p) => s + $"{NameParam(p.nameFlagless)}: {ParamTypeToStr(p)}, ");
			if (paramList != "") paramList = paramList[..^2];
			text.AppendLine($"{indent}extern fn {f.name}({paramList}) {TypeToStr(f.returnType)};");
		}
		if (moduleFunctions.Length > 0) text.AppendLine();


		// These are idiomatic Zig wrappers for the bound functions
		foreach (var f in moduleFunctions)
		{
			SKFunctionRelation rel = SKHeaderData.GetFunctionRelation(m, f);

			string paramList = rel == SKFunctionRelation.Instance ? $"self: *{NameType(m.name)}, " : "";
			if (rel == SKFunctionRelation.Instance) paramList = f.parameters.Skip(1).Aggregate(paramList, (s, p) => s + $"{NameParam(p.nameFlagless)}: {ParamTypeToStrIdiomatic(p)}, ");
			else                                    paramList = f.parameters        .Aggregate(paramList, (s, p) => s + $"{NameParam(p.nameFlagless)}: {ParamTypeToStrIdiomatic(p)}, ");
			if (paramList != "") paramList = paramList[..^2];
			text.AppendLine($"{indent}pub fn {NameMethod(f.name, m.modulePrefix)}({paramList}) {TypeToStrIdiomatic(f.returnType)} {{");
			if (!f.returnType.IsVoid) text.Append($"{indent}\treturn ");
			else                      text.Append($"{indent}\t");
			if (f.returnType.name == "bool32_t") text.Append("0 != ");
			text.Append(f.name+"(");
			string argList = "";
			if (rel == SKFunctionRelation.Instance) argList = "self.*, ";
			if (rel == SKFunctionRelation.Instance) argList = f.parameters.Skip(1).Aggregate(argList, (s, p) => s + $"{ArgToStr(p)}, ");
			else                                    argList = f.parameters        .Aggregate(argList, (s, p) => s + $"{ArgToStr(p)}, ");
			if (argList != "") argList = argList[..^2];
			text.AppendLine($"{argList});");
			text.AppendLine($"{indent}}}");
		}

		if (m.name != "sk")
			text.AppendLine("};");
	}

	///////////////////////////////////////////

	static string ArgToStr(SKParameter p)
	{
		if (p.type.name == "bool32_t") return $"@intFromBool({NameParam(p.nameFlagless)})";
		else return NameParam(p.nameFlagless);
	}

	///////////////////////////////////////////

	static string TypeToStr(SKType type)
	{
		if (type.functionPtr != null)
		{
			string paramList = type.functionPtr.Val.parameters.Aggregate("", (s, p) => s + $"{NameParam(p.nameFlagless)}: {ParamTypeToStr(p)}, ");
			if (paramList != "") paramList = paramList[..^2];
			return $"{(type.isOptional?"?":"")}*const fn ({paramList}) {TypeToStr(type.functionPtr.Val.returnType)}";
		}

		// This needs more work
		string prefix = "";
		if (type.isOptional) prefix += "?";

		if      (type.arraySize1 > 0) prefix += $"[{type.arraySize1}]";
		else if (type.pointerLvl > 0 && (type.name == "char" || type.name == "char8_t" || type.name == "char16_t" || type.name == "char32_t")) prefix += $"[*]";
		else if (type.pointerLvl > 0) prefix += "*";

		if (type.arraySize2 > 0) prefix += $"[{type.arraySize2}]";

		if (type.isConst && type.pointerLvl > 0) prefix += "const ";

		string zigName = NameType(type.name);
		if (type.name == "void" && type.pointerLvl > 0)
		{
			zigName = "anyopaque";
		}

		return $"{prefix}{zigName}";
	}

	///////////////////////////////////////////

	static string TypeToStrIdiomatic(SKType type)
	{
		if (type.name == "bool32_t") return "bool";
		return TypeToStr(type);
	}

	///////////////////////////////////////////

	static string ParamTypeToStr(SKParameter param)
	{
		if (param.type.functionPtr != null)
		{
			string paramList = param.type.functionPtr.Val.parameters.Aggregate("", (s, p) => s + $"{NameParam(p.nameFlagless)}: {ParamTypeToStr(p)}, ");
			if (paramList != "") paramList = paramList[..^2];
			return $"{(param.type.isOptional?"?":"")}*const fn ({paramList}) callconv(.C) {TypeToStr(param.type.functionPtr.Val.returnType)}";
		}

		string prefix = "";
		if      (param.type.isOptional) prefix += "?";

		if      (param.type.arraySize1 > 0)                                 prefix += $"[{param.type.arraySize1}]";
		else if (param.isArrayPtr || param.stringType != SKStringType.None) prefix += "[*]";
		else if (param.type.pointerLvl > 0)                                 prefix += "*";

		if (param.type.arraySize2 > 0) prefix += $"[{param.type.arraySize2}]";

		if (param.type.isConst && param.type.pointerLvl > 0) prefix += "const ";

		string zigName = NameType(param.type.name);
		if (param.type.name == "void" && param.type.pointerLvl > 0)
			zigName = "anyopaque";

		return $"{prefix}{zigName}";
	}

	///////////////////////////////////////////

	static string ParamTypeToStrIdiomatic(SKParameter param)
	{
		if (param.type.name == "bool32_t") return "bool";
		return ParamTypeToStr(param);
	}
}