using System;
using System.IO;

namespace StereoKitDocumenter
{
	enum ExampleType
	{
		CodeSample,
		Document
	}

	class DocExample : IDocItem
	{
		public string info;
		public string category;

		public ExampleType type;
		public string data;

		bool comments;

		bool skipBlanks = true;
		int  skipIndent = -1;
		int  sortIndex  = 0;

		public string Name      => info;
		public string FileName  => Path.Combine(Program.pagesOut, (category.ToLower() == "root" ? "" : category+"/") + info.Replace(' ', '-') + ".md");
		public string UrlName   => $"{{{{site.url}}}}/Pages/{(category.ToLower() == "root" ? "" : category + "/")}{info.Replace(' ', '-')}.html";
		public int    SortIndex => sortIndex;

		public DocExample(ExampleType aType, string aInfo)
		{
			type = aType;
			info = aInfo;
			comments = true;

			if (type == ExampleType.CodeSample) {
				string[] names = info.Split(' ');

				for (int i = 0; i < Program.items.Count; i++)
				{
					if (Array.IndexOf(names, Program.items[i].Name) != -1)
					{
						Program.items[i].AddExample(this);
					}
				}
			} else if (type == ExampleType.Document) {
				string[] words = info.Split(' ');
				category  = words[0];
				sortIndex = int.Parse(words[1]);
				info      = String.Join(" ", words, 2, words.Length - 2);
				Program.items.Add(this);
			}
		}

		public void AddLine(string text)
		{
			if (text.Trim().StartsWith("///"))
			{
				// If we were in some code, finish the section off
				if (!comments) { 
					data += "```\n";
					skipBlanks = true;
					skipIndent = -1;
				}
				comments = true;

				// Add text comments
				string line = text.Substring(text.IndexOf("///") + 3).TrimEnd() + "\n";
				if (line[0] == ' ')
					line = line.Substring(1);
				data += line;
			} else {
				// If we were in comments before, start up a code section
				if (comments)
					data += "```csharp\n";
				comments = false;

				if (!skipBlanks || text.Trim() != "") { 
					skipBlanks = false;
					if (skipIndent == -1)
						skipIndent = text.Length - text.TrimStart(' ', '\t').Length;
					data += (skipIndent < text.Length ? text.Substring(skipIndent) : text) + "\n" ;
				}
			}
		}
		public void End()
		{
			if (!comments)
			{
				data += "```\n";
				skipBlanks = true;
				skipIndent = -1;
				comments = true;
			}
		}

		public override string ToString()
		{

			return $@"---
layout: default
title: {(category.ToLower()=="root"?"":category)} {info}
description: {info}
---

{data}
";
		}

		public void AddExample(DocExample aExample)
		{
			throw new NotImplementedException();
		}
	}
}
