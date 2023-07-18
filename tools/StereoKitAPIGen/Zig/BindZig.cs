// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKitAPIGen;
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

			const bool32 = i32;
			const TextStyle = u32;

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

	///////////////////////////////////////////

	static string NameField(string sourceName)
		=> NameOverrides.TryGet(sourceName, out string custom)
			? custom
			: Tools.SnakeToCamelL(sourceName);

	static string NameParam(string sourceName)
		=> NameField(sourceName);

	///////////////////////////////////////////

	static void BuildEnum(StringBuilder text, SKEnum e)
	{
		string enumName = NameType(e.name);

		Func<string, string> enumItemName = (str) =>
			NameOverrides.TryGet(str, out string custom)
				? custom
				: Tools.SnakeToCamelU(Tools.RemovePrefix(str, e.name));
		Func<string, string> innerItemName = (str) =>
			enumName + "." + (NameOverrides.TryGet(str, out string custom)
				? custom
				: Tools.SnakeToCamelU(Tools.RemovePrefix(str, e.name)));

		text.AppendLine(Tools.ToStrComment(e.comment, "//"));
		text.AppendLine($"pub const {enumName} = enum(c_int) {{");
		foreach (var i in e.items)
		{
			text.AppendLine(Tools.ToStrComment(i.comment, "\t//"));
			text.AppendLine(i.value == null 
				? $"\t{enumItemName(i.name)},"
				: $"\t{enumItemName(i.name)} = {Tools.BuildExpression(i.value, innerItemName)},");
		}
		text.AppendLine($"}};");
	}

	///////////////////////////////////////////

	static void BuildModule(SKHeaderData data, StringBuilder text, SKModule m)
	{
		SKFunction[] moduleFunctions = SKHeaderData.GetModuleFunctions(m, data.functions);

		text.AppendLine($"\r\n///////////////////////////////////////////\r\n");
		text.AppendLine($"pub const {NameType(m.name)} = extern struct {{");
		if (m.isAsset)
			text.AppendLine($"\t_inst: ?*opaque{{}} = null,\r\n");

		foreach (var f in m.fields)
		{
			// Default values need to know a bit more about the type
			//string defaultVal = 
			//	paramType.source.TypeKind == CppTypeKind.Primitive ||
			//	(paramType.source.TypeKind == CppTypeKind.Typedef && ((CppTypedef)paramType.source).ElementType.TypeKind == CppTypeKind.Primitive)
			//		? "0"
			//		: ".{}";
			//text.AppendLine($"{ZigFieldName(f.name) + ":"} {ZigTypeToStr(f.type)} = {defaultVal},");
			text.AppendLine($"\t{NameField(f.name) + ":"} {TypeToStr(f.type)},");
		}
		if (m.fields.Length > 0) text.AppendLine();


		// These are the directly imported functions
		foreach (var f in moduleFunctions)
		{
			string paramList = f.parameters.Aggregate("", (s, p) => s + $"{NameParam(p.nameFlagless)}: {ParamTypeToStr(p)}, ");
			if (paramList != "") paramList = paramList[..^2];
			text.AppendLine($"\textern fn {f.name}({paramList}) {TypeToStr(f.returnType)};");
		}
		if (moduleFunctions.Length > 0) text.AppendLine();


		// These are idiomatic Zig wrappers for the bound functions
		foreach (var f in moduleFunctions)
		{
			SKFunctionRelation rel = SKHeaderData.GetFunctionRelation(m, f);

			string paramList = rel == SKFunctionRelation.Instance ? $"self: *{NameType(m.name)}, " : "";
			if (rel == SKFunctionRelation.Instance) paramList = f.parameters.Skip(1).Aggregate(paramList, (s, p) => s + $"{NameParam(p.nameFlagless)}: {ParamTypeToStr(p)}, ");
			else                                    paramList = f.parameters        .Aggregate(paramList, (s, p) => s + $"{NameParam(p.nameFlagless)}: {ParamTypeToStr(p)}, ");
			if (paramList != "") paramList = paramList[..^2];
			text.AppendLine($"\tpub fn {NameMethod(f.name, m.modulePrefix)}({paramList}) {TypeToStr(f.returnType)} {{");
			if (!f.returnType.IsVoid) text.Append("\t\treturn ");
			else                      text.Append("\t\t");
			text.Append(f.name+"(");
			string argList = "";
			if (rel == SKFunctionRelation.Instance) argList = "self._inst.?, ";
			if (rel == SKFunctionRelation.Instance) argList = f.parameters.Skip(1).Aggregate(argList, (s, p) => s + $"{NameParam(p.nameFlagless)}, ");
			else                                    argList = f.parameters        .Aggregate(argList, (s, p) => s + $"{NameParam(p.nameFlagless)}, ");
			if (argList != "") argList = argList[..^2];
			text.AppendLine($"{argList});");
			text.AppendLine("\t}");
		}

		text.AppendLine("};");
	}

	///////////////////////////////////////////

	static string TypeToStr(SKType type)
	{
		// This needs more work
		string prefix = "";
		if      (type.arraySize1 > 0) prefix += $"[{type.arraySize1}]";
		else if (type.pointerLvl > 0 && (type.name == "char" || type.name == "char8_t" || type.name == "char16_t" || type.name == "char32_t")) prefix += $"[*]";
		else if (type.pointerLvl > 0) prefix += "*";

		if (type.arraySize2 > 0) prefix += $"[{type.arraySize2}]";

		if (type.isConst) prefix += "const ";

		string zigName = NameType(type.name);
		if (type.name == "void" && type.pointerLvl > 0)
		{
			zigName = "anyopaque";
			prefix = "?*";
		}

		return $"{prefix}{zigName}";
	}

	///////////////////////////////////////////

	static string ParamTypeToStr(SKParameter param)
	{
		// This needs more work
		//string zigName = NameType(param.name);
		//if (type.arraySize1 > 0) return $"[{type.arraySize1}]{(type.isConst ? "const " : "")}{zigName}";
		//else return (type.pointerLvl > 0 ? "*" : "") + zigName;

		string prefix = "";
		if      (param.isOptional) prefix += "?";

		if      (param.type.arraySize1 > 0)                                 prefix += $"[{param.type.arraySize1}]";
		else if (param.isArrayPtr || param.stringType != SKStringType.None) prefix += "[*]";
		else if (param.type.pointerLvl > 0)                                 prefix += "*";

		if (param.type.arraySize2 > 0) prefix += $"[{param.type.arraySize2}]";

		if (param.type.isConst) prefix += "const ";

		string zigName = NameType(param.type.name);

		return $"{prefix}{NameType(param.type.name)}";
	}

	/*public static string TypeToName(SKType type)
	{
		if      (type.special == SKSpecialType.VoidPtr) return "?*anyopaque";
		else if (type.special == SKSpecialType.Utf8   ) return type.constant ? "[*]const u8"  : "[*]u8";
		else if (type.special == SKSpecialType.Utf16  ) return type.constant ? "[*]const u16" : "[*]u16";
		else if (type.special == SKSpecialType.Ascii  ) return type.constant ? "[*]const u8"  : "[*]u8";
		else if (type.special == SKSpecialType.FnPtr  ) {
			string result = "*const fn(";
			CppFunctionType fn = type.source as CppFunctionType;
			for (int i = 0; i < fn.Parameters.Count; i++)
			{
				SKType t = SKType.Create(fn.Parameters[i].Type, fn.Parameters[i].Name);
				result += $"{NameOverrides.Check(fn.Parameters[i].Name)}: {TypeToName(t)}";
				if (i < fn.Parameters.Count-1) result += ", "; 
			}
			SKType r = SKType.Create(fn.ReturnType, "");
			return result+ ") callconv(.C) " + TypeToName(r);
		}
		else return type.array
			? $"[{(type.fixedArray != 0? type.fixedArray:"*")}]{(type.constant?"const ":"")}{CSTypes.SnakeToCamel(type.raw, true, 0)}"
			: (type.constant?"":"")+(type.pointer > 0 ? "*":"")+CSTypes.SnakeToCamel(type.raw, true, 0);
	}*/
}