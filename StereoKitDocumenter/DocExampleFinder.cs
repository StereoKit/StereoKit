using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitDocumenter
{
    static class DocExampleFinder
    {
        static List<DocExample> examples = new List<DocExample>();

        public static void FindExamples(string root)
        {
            string[] files = Directory.GetFiles(root, "*.cs", SearchOption.AllDirectories);

            for (int i = 0; i < files.Length; i++)
            {
                Console.WriteLine(files[i]);
                StreamReader reader = new StreamReader(files[i]);
                ParseFile(reader.ReadToEnd());
                reader.Close();
            }

            for (int i = 0; i < examples.Count; i++)
            {
                Console.WriteLine(examples[i]);
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
                        curr = new DocExample(ExampleType.Document, lines[i].Substring(lines[i].LastIndexOf(':')).Trim());
                        examples.Add(curr);
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
        }
    }
}
