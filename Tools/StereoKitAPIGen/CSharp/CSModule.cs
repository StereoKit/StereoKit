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

	static void BuildRawFunction(StringBuilder fnText, Dictionary<string, string> delegateText, CppFunction fn)
	{
		fnText.Append("[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ");
		fnText.Append($"{CSTypes.TypeName(fn.ReturnType, "", null).RawName,-20}{fn.Name,-20}(");
		bool first = true;
		int i = 0;
		foreach (var p in fn.Parameters)
		{
			if (first) { first = false; }
			else { fnText.Append(", "); }

			// Something kinda weird happening here with function pointer
			// params, but this code seems to work just fine.
			if (p.Type.TypeKind == CppTypeKind.Pointer && ((CppPointerType)p.Type).ElementType.TypeKind == CppTypeKind.Function)
			{
				CppFunctionType fnParam  = (CppFunctionType)((CppPointerType)p.Type).ElementType;
				CppParameter    subParam = fnParam.Parameters[i];
				fnText.Append($"{CSTypes.TypeName(subParam.Type, subParam.Name, delegateText).RawName} {subParam.Name}");
			}
			else
			{
				fnText.Append($"{CSTypes.TypeName(p.Type, p.Name, delegateText).RawName} {p.Name}");
			}
			i+=1;
		}
		fnText.AppendLine(");");
	}

	///////////////////////////////////////////

	public void BuildRawModule(StringBuilder builder, Dictionary<string, string> delegates, string indent)
	{
		builder.Append($"\r\n{indent}///////////////////////////////////////////\r\n\r\n");
		for (int i = 0; i < functions.Count; i++)
		{
			builder.Append(indent);
			BuildRawFunction(builder, delegates, functions[i]);
		}
	}
}