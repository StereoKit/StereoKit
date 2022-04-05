using System;
using System.Collections.Generic;
using System.IO;
using System.Xml;

namespace StereoKitDocumenter
{
	class Program
	{
		public const string xmlDocs      = "../../../../bin/StereoKit.xml";
		public const string pagesOut     = "../../../../docs/Pages/";
		public const string samplesProj  = "../../../../Examples/StereoKitTest/";
		public const string referenceOut = pagesOut+"Reference/";

		public static List<DocClass>  classes = new List<DocClass>();
		public static List<DocMethod> methods = new List<DocMethod>();
		public static List<DocField>  fields  = new List<DocField>();
		public static List<IDocItem>  items   = new List<IDocItem>();
		public static List<DocInheritMethod> inheritMethods = new List<DocInheritMethod>();
		public static List<DocInheritField>  inheritFields  = new List<DocInheritField>();

		public static DocClass GetClass(string name) {
			DocClass result = classes.Find((a) => a.name == name);
			if (result == null)
				throw new Exception($"Couldn't find a class with docs by name of '{name}'. Are you missing a <Summary> for this class?");
			return result;
		}
		public static bool TryGetClass(string name, out DocClass result)
		{
			result = classes.Find((a) => a.name == name);
			return result != null;
		}

		static void Main(string[] args)
		{
			int result = RunSKTests();
			if (result != 0)
				Environment.Exit(result);

			Console.WriteLine("Building doc pages...");
			ScrapeData();
			WriteDocsToFile();
			Console.WriteLine("Done!");
		}

		private static void WriteDocsToFile()
		{
			for (int i = 0; i < items.Count; i++)
			{
				Directory.CreateDirectory(Path.GetDirectoryName(items[i].FileName));
				StreamWriter writer = new StreamWriter(items[i].FileName);
				writer.Write(items[i].ToString());
				writer.Close();
			}

			classes.Sort((a, b) => a.name.CompareTo(b.name));
			{
				StreamWriter writer = new StreamWriter(pagesOut + "data.js");
				writer.Write(WriteIndex());
				writer.Close();
			}
		}

		private static void ScrapeData()
		{
			XmlReader reader = XmlReader.Create(xmlDocs);
			while (reader.ReadToFollowing("member"))
			{
				string    name      = reader.GetAttribute("name");
				XmlReader member    = reader.ReadSubtree();
				string    type      = name[0].ToString();
				string    signature = name.Substring(2);

				if (type == "T")
				{
					ReadClass(signature, reader.ReadSubtree());
				}
				else if (type == "M")
				{
					ReadMethod(signature, reader.ReadSubtree());
				}
				else if (type == "F" || type == "P")
				{
					ReadField(signature, reader.ReadSubtree());
				}
			}

			// Check inheritdoc references, and update their values
			inheritFields .ForEach(f=>f.Resolve(fields));
			inheritMethods.ForEach(f=>f.Resolve(methods));

			DocExampleFinder.FindExamples(samplesProj);
		}

		private static int RunSKTests()
		{
			var testInfo = new System.Diagnostics.ProcessStartInfo();
			testInfo.FileName         = "cmd.exe";
			testInfo.Arguments        = "/C StereoKitTest.exe -test";
			testInfo.UseShellExecute  = false;
			#if DEBUG
			testInfo.WorkingDirectory = "../../../../bin/x64_Debug/StereoKitTest/";
			#else
			testInfo.WorkingDirectory = "../../../../bin/x64_Release/StereoKitTest/";
			#endif
			var process = System.Diagnostics.Process.Start(testInfo);
			process.WaitForExit();
			Console.WriteLine("Ran StereoKit tests! Result: " + process.ExitCode);

			return process.ExitCode;
		}

		static void ReadClass(string signature, XmlReader reader)
		{
			DocClass result = new DocClass();

			// Get names
			string[] segs = signature.Split('.');
			result.name = segs[segs.Length-1];

			// Read properties
			while (reader.Read())
			{
				switch (reader.Name.ToLower())
				{
					case "summary": result.summary = StringHelper.CleanMultiLine(reader.ReadElementContentAsString().Trim()); break;
				}
			}

			classes.Add(result);
			items.Add(result);
		}

		static void ReadField(string signature, XmlReader reader)
		{
			// Get names
			string[] segs = signature.Split('(');
			string nameSignature = segs[0];
			string paramSignature = segs.Length > 1 ? segs[1] : "";
			segs = nameSignature.Split('.');

			DocField result = new DocField(GetClass(segs[segs.Length-2]), segs[segs.Length-1]);

			// Read properties
			while (reader.Read())
			{
				switch (reader.Name.ToLower())
				{
					case "summary": result.summary = StringHelper.CleanMultiLine(reader.ReadElementContentAsString().Trim()); break;
					case "inheritdoc": {
						string reference = reader.GetAttribute("cref").Trim();
						int    start     = reference.IndexOf('.');
						start = start == -1 ? 0 : start+1;
						result.summary = $"See `{reference.Substring(start)}`";
						inheritFields.Add(new DocInheritField(result, reader.GetAttribute("cref")));
					} break;
				}
			}

			fields.Add(result);
			items .Add(result);
		}

		static void ReadMethod(string signature, XmlReader reader)
		{
			// Get names
			string[] segs = signature.Split('(');
			string nameSignature  = segs[0];
			string paramSignature = segs.Length>1?segs[1] :"";
			segs = nameSignature.Split('.');
			if (paramSignature.Length > 0)
			{
				int end = paramSignature.IndexOf(')');
				if (end == -1) end = paramSignature.Length - 1;
				paramSignature = paramSignature.Substring(0, end);
			}

			DocMethod method = methods.Find(a => a.name == segs[segs.Length-1] && a.parent.name == segs[segs.Length-2]);
			if (method == null)
			{
				method = new DocMethod(GetClass(segs[segs.Length-2]), segs[segs.Length-1]);
				methods.Add(method);
				items.Add(method);
			}

			DocMethodOverload variant = method.AddOverload(paramSignature);

			// Read properties
			while (reader.Read())
			{
				switch(reader.Name.ToLower())
				{
					case "summary": variant.summary = StringHelper.CleanMultiLine(reader.ReadElementContentAsString().Trim()); break;
					case "returns": variant.returns = StringHelper.CleanMultiLine(reader.ReadElementContentAsString().Trim()); break;
					case "param": {
						DocParam p = new DocParam();
						p.name    = reader.GetAttribute("name");
						p.summary = reader.ReadElementContentAsString().Trim();
						variant.parameters.Add(p);
					} break;
					case "inheritdoc": {
						inheritMethods.Add(new DocInheritMethod(variant, reader.GetAttribute("cref")));
					} break;
				}
			}
		}

		static string WriteIndex()
		{
			DocIndexFolder root      = new DocIndexFolder("Pages");
			DocIndexFolder reference = new DocIndexFolder("Reference");
			root.folders.Add(reference);

			for (int i = 0; i < classes.Count; i++)
			{
				DocIndexFolder classFolder = new DocIndexFolder(classes[i].Name);
				reference.folders.Add(classFolder);

				// Enums don't need child items, it's a little much.
				if (classes[i].IsEnum) continue;

				for (int f = 0; f < classes[i].fields.Count; f++)
				{
					classFolder.folders.Add(new DocIndexFolder(classes[i].fields[f].name));
				}

				for (int m = 0; m < classes[i].methods.Count; m++)
				{
					classFolder.folders.Add(new DocIndexFolder(classes[i].methods[m].ShowName));
				}
			}

			DocExampleFinder.examples.Sort((a,b)=>a.info.CompareTo(b.info));
			for (int e = 0; e < DocExampleFinder.examples.Count; e++)
			{
				DocExample ex = DocExampleFinder.examples[e];
				if (ex.type == ExampleType.Document)
				{
					DocIndexFolder folder = null;
					if (ex.category.ToLower() == "root")
						folder = root;
					else
						folder = root.folders.Find((a) => a.name == ex.category);
					if (folder == null)
					{
						folder = new DocIndexFolder(ex.category, ex.SortIndex);
						root.folders.Add(folder);
					}
					folder.folders.Add(new DocIndexFolder( ex.Name, ex.SortIndex));
				}
			}
			for (int i = 0; i < root.folders.Count; i++)
			{
				if (root.folders[i] == reference) continue;
				root.folders[i].folders.Sort((a, b) => a.order - b.order);
			}

			root     .folders.Sort((a,b)=>string.Compare(a.name,b.name));
			reference.folders.Sort((a,b)=>string.Compare(a.name,b.name));

			return root.ToString();
		}
	}
}
