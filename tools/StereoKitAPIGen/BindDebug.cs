using System;
using System.Linq;

namespace StereoKitAPIGen;

class BindDebug
{
	public static void Bind(SKHeaderData data)
	{
		Console.WriteLine("// DEBUG BINDINGS");
		Console.WriteLine("// These are for verification of parsing, or reference, nothing more");

		Console.WriteLine("\n//////////// Enums ////////////\n");
		PrintEnums(data);

		Console.WriteLine("\n//////////// Structs ////////////\n");
		PrintModules(data, (m) => m.fields.Length != 0);

		Console.WriteLine("\n//////////// Assets ////////////\n");
		PrintModules(data, (m) => m.fields.Length == 0 && m.isAsset == true);

		Console.WriteLine("\n//////////// Static Modules ////////////\n");
		PrintModules(data, (m) => m.fields.Length == 0 && m.isAsset == false);
	}

	///////////////////////////////////////////

	static void PrintEnums(SKHeaderData data)
	{
		foreach (var e in data.enums)
		{
			Console.WriteLine(ToStrComment(e.comment, "//"));
			Console.WriteLine($"{(e.isBitflag?"flag":"enum")} {e.name}");
			foreach (var i in e.items)
			{
				Console.WriteLine(ToStrComment(i.comment, "  //"));
				Console.WriteLine(i.value == null ? $"  {i.name}" : $"  {i.name} = {i.value}");
			}
			Console.WriteLine();
		}
	}

	///////////////////////////////////////////

	static void PrintModules(SKHeaderData data, Func<SKModule, bool> printIf)
	{
		foreach (var m in data.modules)
		{
			if (!printIf(m)) continue;

			Console.WriteLine($"module {m.name}");
			foreach (var f in m?.fields)
				Console.WriteLine($"  {ToStrType(f.type),-20} : {f.name}");
			if (m.fields.Length != 0) Console.WriteLine();

			SKFunction[] moduleFunctions = SKHeaderData.GetModuleFunctions(m, data.functions);

			int ct = PrintFunctions(moduleFunctions, SKFunctionRelation.Creation, "ctr    ");
			if (ct != 0) Console.WriteLine();

			ct = PrintFunctions(moduleFunctions, SKFunctionRelation.Instance, "inst   ");
			if (ct != 0) Console.WriteLine();

			ct = PrintFunctions(moduleFunctions, SKFunctionRelation.Static, "static ");
			if (ct != 0) Console.WriteLine();
		}
	}

	///////////////////////////////////////////

	static int PrintFunctions(SKFunction[] fns, SKFunctionRelation relation, string prefix)
	{
		int count = 0;
		foreach (var f in fns)
		{
			SKFunctionRelation curr = SKHeaderData.GetFunctionRelation(f.module, f);
			if (curr != relation) continue;

			string paramList = f.parameters.Aggregate("", (s, p) => s + $"{ToStrType(p.type)} {p.nameFlagless}, ");
			if (paramList != "")
				paramList = paramList[..^2];
			Console.WriteLine($"  {prefix}{ToStrType(f.returnType),-21} {f.name,-30}({paramList})");

			count++;
		}
		return count;
	}

	///////////////////////////////////////////

	static string ToStrType(SKType type)
	{
		string result = type.name;
		if (type.arraySize1 != 0) result += $"[{type.arraySize1}]";
		if (type.arraySize2 != 0) result += $"[{type.arraySize2}]";
		if (type.isConst)         result  = "const " + result;
		if (type.pointerLvl == 1) result += "*";
		if (type.pointerLvl == 2) result += "**";
		if (type.pointerLvl == 3) result += "***";
		return result;
	}

	///////////////////////////////////////////

	static string ToStrComment(string comment, string prefix, int columns = 80)
	{
		string   lineStart  = $"{prefix}";
		string   result     = "";
		string[] paragraphs = comment.Split("\n");
		for (int i = 0; i < paragraphs.Length; i++)
		{
			string[] words = paragraphs[i].Split(" ");
			string   line  = lineStart;
			for (int w = 0; w < words.Length; w++)
			{
				if (line.Length + words[w].Length+1 > columns)
				{
					result += line;
					line = "\n" + lineStart;
				}
				line += " " + words[w];
			}
			result += line;
			if (i<paragraphs.Length-1)
				result += $"\n{prefix}\n";
		}
		return result;
	}
}
