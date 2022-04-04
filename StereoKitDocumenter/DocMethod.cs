using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace StereoKitDocumenter
{
	class DocMethod : IDocItem
	{
		public string   name;
		public DocClass parent;
		public bool     isOp;
		public List<DocExample>        examples  = new List<DocExample>();
		public List<DocMethodOverload> overloads = new List<DocMethodOverload>();

		public bool   IsStatic => overloads.Count > 0 ? overloads[0].IsStatic : throw new Exception("Calling this too early?");
		public string Name     => $"{parent.Name}.{ShowName}";
		public string FileName => Path.Combine(Program.referenceOut, parent.Name + "/" + PathName + ".md");
		public string UrlName  => $"{{{{site.url}}}}/Pages/Reference/{parent.Name}/{PathName}.html";
		public string ShowName { get {
			if      (name == "#ctor")        return parent.Name;
			else if (name.StartsWith("op_")) return OpName(name);
			else if (name.Contains  ('`'))   return name.Substring(0, name.IndexOf('`'));
			else                             return name;
		} }
		public string PathName { get {
			if      (name == "#ctor")        return parent.Name;
			else if (name.Contains  ('`'))   return name.Substring(0, name.IndexOf('`'));
			else                             return name;
		} }

		public DocMethod(DocClass aParent, string aName)
		{
			parent = aParent;
			name   = aName;
			isOp   = name.StartsWith("op_");
			parent.methods.Add(this);
		}

		public DocMethodOverload AddOverload(string aSignature)
		{
			DocMethodOverload result = new DocMethodOverload(this, aSignature);
			overloads.Add(result);
			return result;
		}

		private string OpName(string op)
		{
			switch(op)
			{
				case "op_Addition"     : return "+";
				case "op_Division"     : return "/";
				case "op_Multiply"     : return "*";
				case "op_Subtraction"  : return "-";
				case "op_UnaryNegation": return "-";
				case "op_Implicit"     : return "Implicit Conversions";
			}
			return op;
		}

		public void AddExample(DocExample aExample) => examples.Add(aExample);

		public override string ToString()
		{
			string exampleText = "";
			if (examples.Count > 0) {
				exampleText = "\n\n## Examples\n\n";
				for (int i = 0; i < examples.Count; i++) {
					exampleText += examples[i].data;
				}
			}

			return $@"---
layout: default
title: {parent.Name}.{ShowName}
description: {StringHelper.CleanForDescription(overloads[0].summary)}
---
# [{parent.Name}]({parent.UrlName}).{ShowName}

{string.Join("",overloads.Select(a=>a.ToString()).ToArray())}

{exampleText}
";

		}

	}
}
