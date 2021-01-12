using System;
using System.Collections.Generic;
using System.IO;

namespace StereoKitDocumenter
{
	static class DocExampleFinder
	{
		public static List<DocExample> examples = new List<DocExample>();

		public static void FindExamples(string root)
		{
			string[] files = Directory.GetFiles(root, "*.cs", SearchOption.AllDirectories);

			for (int i = 0; i < files.Length; i++)
			{
				StreamReader reader = new StreamReader(files[i]);
				ParseFile(reader.ReadToEnd());
				reader.Close();
			}
		}

		public static void ParseFile(string contents)
		{
			string[] lines = contents.Split('\n');
			DocExample curr = null;
			for (int i = 0; i < lines.Length; i++)
			{
				if (curr == null) { 
					string trim = lines[i].Trim();
					if (trim.StartsWith("///") && lines[i].Contains(":CodeSample:")) {
						curr = new DocExample( ExampleType.CodeSample, lines[i].Substring(lines[i].LastIndexOf(':')).Trim());
						examples.Add(curr);
					} else if (trim.StartsWith("///") && lines[i].Contains(":CodeDoc:")) {
						string docName = lines[i].Substring(lines[i].LastIndexOf(':') + 1).Trim();
						curr = FindDoc(docName);
						if (curr == null) { 
							curr = new DocExample(ExampleType.Document, docName);
							examples.Add(curr);
						}
					}
				} else {
					string trim = lines[i].Trim();
					if (trim.StartsWith("///") && lines[i].Contains(":End:"))
					{
						curr.End();
						curr = null;
					} else {
						curr.AddLine(lines[i]);
					}
				}
			}
			if (curr != null) { 
				Console.WriteLine("Missing an :End: in documentation code sample for " + curr.Name + "!");
			}
		}

		private static DocExample FindDoc(string docName)
		{
			int    split    = docName.IndexOf(' ');
			string category = docName.Substring(0, split);
			string info     = docName.Substring(split + 1);

			for (int i = 0; i < examples.Count; i++)
			{
				if (examples[i].category == category &&
					examples[i].info == info)
					return examples[i];
			}
			return null;
		}
	}
}
