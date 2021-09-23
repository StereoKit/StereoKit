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
		public List<DocExample>        examples  = new List<DocExample>();
		public List<DocMethodOverload> overloads = new List<DocMethodOverload>();

		public bool   IsStatic => overloads.Count > 0 ? overloads[0].IsStatic : throw new Exception("Calling this too early?");
		public string ShowName => name == "#ctor" ? parent.Name : name;
		public string Name     => $"{parent.Name}.{ShowName}";
		public string FileName => Path.Combine(Program.referenceOut, parent.Name + "/" + ShowName + ".md");
		public string UrlName  => $"{{{{site.url}}}}/Pages/Reference/{parent.Name}/{ShowName}.html";

		public DocMethod(DocClass aParent, string aName)
		{
			parent = aParent;
			name   = aName;
			parent.methods.Add(this);
		}

		public DocMethodOverload AddOverload(string aSignature)
		{
			DocMethodOverload result = new DocMethodOverload(this, aSignature);
			overloads.Add(result);
			return result;
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
