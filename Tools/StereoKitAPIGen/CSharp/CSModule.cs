using CppAst;
using System.Collections.Generic;
using System.Text;

class CSModule
{
	public string            module;
	public List<CppFunction> functions = new List<CppFunction>();

	public CSModule(string module) => this.module = module;
	
	public void AddFunction(CppFunction fn)
	{
		functions.Add(fn);
	}

	///////////////////////////////////////////

	static void BuildRawFunction(StringBuilder fnText, CppFunction fn)
	{
		fnText.Append("[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ");
		fnText.Append($"{CSTypes.TypeName(fn.ReturnType, "").RawName,-20}{fn.Name,-20}(");
		bool first = true;
		foreach (var p in fn.Parameters)
		{
			if (first) { first = false; }
			else { fnText.Append(", "); }

			fnText.Append($"{CSTypes.TypeName(p.Type, p.Name).RawName} {p.Name}");
		}
		fnText.AppendLine(");");
	}

	///////////////////////////////////////////

	public void BuildRawModule(StringBuilder builder, string indent)
	{
		builder.Append($"\r\n{indent}///////////////////////////////////////////\r\n\r\n");
		for (int i = 0; i < functions.Count; i++)
		{
			builder.Append(indent);
			BuildRawFunction(builder, functions[i]);
		}
	}
}