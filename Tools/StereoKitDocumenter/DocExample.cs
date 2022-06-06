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
		int skipIndent = -1;
		int sortIndex = 0;

		public string Name => info;
		public string FileName => Path.Combine(Program.options.PagesOut, (category.ToLower() == "root" ? "" : category + "/") + info.Replace(' ', '-') + ".md");
		public string UrlName => $"{{{{site.url}}}}/Pages/{(category.ToLower() == "root" ? "" : category + "/")}{info.Replace(' ', '-')}.html";
		public int SortIndex => sortIndex;

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
				category = words[0];
				sortIndex = int.Parse(words[1]);
				info = String.Join(" ", words, 2, words.Length - 2);
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
					data += (skipIndent < text.Length ? text.Substring(skipIndent) : text) + "\n";
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
title: {info}
description: {GetSummary(data)}
---

{data}
";
		}

		private string GetSummary(string text)
		{
			int newline = 0;
			int i = 0;
			int start = 0;
			text = text.Trim();
			if (text.StartsWith("#"))
			{
				while(text[start] != '\n' && start < text.Length) start++;
			}
			while (char.IsWhiteSpace(text[start]) && start < text.Length) start++;

			for (i=start; i < text.Length && i-start < 140; i++)
			{
				if      (text[i] == '\n') newline++;
				else if (text[i] == '\r') { }
				else if (text[i] == '#' ) break;
				else                      newline = 0;
			}
			return StringHelper.CleanForDescription(text.Substring(start, i))
				+ (start + 140 < text.Length ? "...":"");
		}

		public void AddExample(DocExample aExample)
		{
			throw new NotImplementedException();
		}
	}
}
