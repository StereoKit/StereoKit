using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;

namespace StereoKitDocumenter
{
	class DocField : IDocItem
	{
		public string name;
		public string summary;
		public DocClass parent;
		public List<DocExample> examples = new List<DocExample>();

		public bool IsStatic { get; private set;}

		public DocField(DocClass aParent, string aName)
		{
			parent = aParent;
			name   = aName;
			parent.fields.Add(this);
		}

		public string Name     => $"{parent.Name}.{name}";
		public string FileName => Path.Combine(Program.referenceOut, parent.Name + "/" + name + ".md");
		public string UrlName  => $"{{{{site.url}}}}/Pages/Reference/{parent.Name}/{name}.html";

		public Type GetFieldType(Type classType) {
			Type result = classType.GetField(name)?.FieldType;
			if (result == null)
				result = classType.GetProperty(name)?.PropertyType;
			return result;
		}
		public bool GetStatic(Type classType)
		{
			FieldInfo fInfo = classType.GetField(name);
			if (fInfo != null)
				return fInfo.IsStatic;

			PropertyInfo pInfo = classType.GetProperty(name);
			if (pInfo != null)
				return pInfo.GetAccessors(true).Any(x => x.IsStatic);

			return false;
		}

		public void AddExample(DocExample aExample) => examples.Add(aExample);

		public override string ToString()
		{
			Type   classType   = parent.ClassType;
			Type   fieldType   = GetFieldType(classType);
			string exampleText = "";
			if (examples.Count > 0) {
				exampleText = "\n\n## Examples\n\n";
				for (int i = 0; i < examples.Count; i++) {
					exampleText += examples[i].data;
				}
			}

			string signature = (GetStatic(classType) ? "static " : "") + $"{StringHelper.TypeName(fieldType.Name)} {name}";
			PropertyInfo pInfo = classType.GetProperty(name);
			if (pInfo != null)
				signature += $"{{ {(pInfo.CanRead?"get ":"")}{(pInfo.CanWrite ? "set " : "")}}}";

			return $@"---
layout: default
title: {parent.Name}.{name}
description: {StringHelper.CleanForDescription(summary)}
---
# [{parent.Name}]({parent.UrlName}).{name}

<div class='signature' markdown='1'>
{signature}
</div>

## Description
{summary}
{exampleText}
";

		}
	}
}
