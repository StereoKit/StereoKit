using CppAst;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class ZigModule
{
	public string            module;
	public List<CppFunction> functions = new List<CppFunction>();
	public List<CppField>    fields    = new List<CppField>();
	public bool              isAsset   = false;

	public ZigModule(string module) => this.module = module;
	
	public void AddFunction(CppFunction fn) => functions.Add(fn);
	public void AddField   (CppField field) => fields.Add(field);

	///////////////////////////////////////////

	void BuildRawFunction(StringBuilder fnText, Dictionary<string, string> delegateText, CppFunction fn)
	{
		SKType returnType = SKType.Create(fn.ReturnType, "");
		string parameters = "";
		bool   first = true;
		int    i     = 0;
		foreach (var p in fn.Parameters)
		{
			if (!first) { parameters += ", "; }
			SKType paramType;

			// Something kinda weird happening here with function pointer
			// params, but this code seems to work just fine.
			if (p.Type.TypeKind == CppTypeKind.Pointer && ((CppPointerType)p.Type).ElementType.TypeKind == CppTypeKind.Function)
			{
				CppFunctionType fnParam  = (CppFunctionType)((CppPointerType)p.Type).ElementType;
				CppParameter    subParam = fnParam.Parameters[i];
				paramType = SKType.Create(subParam.Type, subParam.Name);
				parameters += $"{NameOverrides.Check(subParam.Name)}: {BindZig.TypeToName(paramType)}";
			}
			else
			{
				paramType = SKType.Create(p.Type, p.Name);
				parameters += $"{NameOverrides.Check(p.Name)}: {BindZig.TypeToName(paramType)}";
			}
			i += 1;
			first = false;
		}
		string line = $"extern fn {fn.Name,-17}({parameters,-40}) {BindZig.TypeToName(returnType)};";
		fnText.AppendLine(line);
	}
	void BuildZigFunction(StringBuilder fnText, Dictionary<string, string> delegateText, CppFunction fn)
	{
		SKType        returnType = SKType.Create(fn.ReturnType, "");
		SKSpecialType textType   = returnType.special;

		string parameters = "";
		bool   first = true;
		int    i     = 0;
		foreach (var p in fn.Parameters)
		{
			if (!first) { parameters += ", "; }
			SKType paramType;

			// Something kinda weird happening here with function pointer
			// params, but this code seems to work just fine.
			if (p.Type.TypeKind == CppTypeKind.Pointer && ((CppPointerType)p.Type).ElementType.TypeKind == CppTypeKind.Function)
			{
				CppFunctionType fnParam  = (CppFunctionType)((CppPointerType)p.Type).ElementType;
				CppParameter    subParam = fnParam.Parameters[i];
				paramType = SKType.Create(subParam.Type, subParam.Name);
				if (first && string.Compare(BindZig.TypeToName(paramType), module, true) == 0 && string.Compare(p.Name, module, true) == 0)
					parameters += $"self: *{module}";
				else parameters += $"{CSTypes.SnakeToCamel(subParam.Name, false, 0)}: {BindZig.TypeToName(paramType)}";
			}
			else
			{
				paramType = SKType.Create(p.Type, p.Name);
				if (first && string.Compare(BindZig.TypeToName(paramType), module, true) == 0 && string.Compare(p.Name, module, true) == 0)
					parameters += $"self: *{module}";
				else parameters += $"{CSTypes.SnakeToCamel(p.Name, false, 0)}: {BindZig.TypeToName(paramType)}";
			}
			if (paramType.special != SKSpecialType.None) textType = paramType.special;
			i += 1;
			first = false;
		}

		string fnName = CSTypes.SnakeToCamelFn(fn.Name, false, module.Length);
		string line = $"pub fn {fnName,-20}({parameters,-40}) {BindZig.TypeToName(returnType),-20} {{ ";

		if (fn.ReturnType.TypeKind != CppTypeKind.Primitive || ((CppPrimitiveType)fn.ReturnType).Kind != CppPrimitiveKind.Void)
			line += "return ";

		//fnText.AppendLine(line);
		//return;

		line  += $"{fn.Name}(";
		first = true;
		i     = 0;
		foreach (var p in fn.Parameters)
		{
			if (!first) { line += ", "; }
			SKType paramType;

			// Something kinda weird happening here with function pointer
			// params, but this code seems to work just fine.
			if (p.Type.TypeKind == CppTypeKind.Pointer && ((CppPointerType)p.Type).ElementType.TypeKind == CppTypeKind.Function)
			{
				CppFunctionType fnParam  = (CppFunctionType)((CppPointerType)p.Type).ElementType;
				CppParameter    subParam = fnParam.Parameters[i];
				paramType = SKType.Create(subParam.Type, subParam.Name);
				if (first && string.Compare(BindZig.TypeToName(paramType), module, true) == 0 && string.Compare(subParam.Name, module, true) == 0)
					line += "self._inst.?";
				else line += $"{CSTypes.SnakeToCamel(subParam.Name, false, 0)}";
			}
			else
			{
				paramType = SKType.Create(p.Type, p.Name);
				if (first && string.Compare(BindZig.TypeToName(paramType), module, true) == 0 && string.Compare(p.Name, module, true) == 0)
					line += "self._inst.?";
				else line += $"{CSTypes.SnakeToCamel(p.Name, false, 0)}";
			}
			if (paramType.special != SKSpecialType.None) textType = paramType.special;
			i+=1;
			first = false;
		}
		line += "); }";
		fnText.AppendLine(line);
	}

	///////////////////////////////////////////

	public void BuildRawModule(StringBuilder builder, Dictionary<string, string> delegates, string indent)
	{
		builder.Append($"\r\n{indent}///////////////////////////////////////////\r\n\r\n");
		builder.AppendLine($"{indent}pub const {CSTypes.SnakeToCamel(module, true, 0)} = extern struct {{");
		if (isAsset)
			builder.AppendLine($"{indent}    _inst: ?*opaque{{}} = null,\r\n");
		for (int i = 0; i < fields.Count; i++)
		{
			builder.Append(indent);
			builder.Append("    ");

			SKType paramType = SKType.Create(fields[i].Type, fields[i].Name);
			string defaultVal = 
				 paramType.source.TypeKind == CppTypeKind.Primitive || 
				(paramType.source.TypeKind == CppTypeKind.Typedef && ((CppTypedef)paramType.source).ElementType.TypeKind == CppTypeKind.Primitive) 
				? "0"
				: ".{}";
			builder.AppendLine($"{CSTypes.SnakeToCamel(fields[i].Name, false, 0)+":",-20} {BindZig.TypeToName(paramType)} = {defaultVal},");
		}
		if (fields.Count != 0 && functions.Count != 0) builder.AppendLine();
		for (int i = 0; i < functions.Count; i++)
		{
			builder.Append(indent);
			builder.Append("    ");
			BuildRawFunction(builder, delegates, functions[i]);
		}
		if (functions.Count != 0) builder.AppendLine();
		for (int i = 0; i < functions.Count; i++)
		{
			builder.Append(indent);
			builder.Append("    ");
			BuildZigFunction(builder, delegates, functions[i]);
		}
		builder.AppendLine($"{indent}}};");
	}
}