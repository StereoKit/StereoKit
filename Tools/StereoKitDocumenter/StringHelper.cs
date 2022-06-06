using System.Collections.Generic;
using System.Linq;
using System.Xml;

namespace StereoKitDocumenter
{
	static class StringHelper
	{
		public static string CleanForDescription(string text)
		{
			return text.Replace('\n', ' ')
						.Replace('\r', ' ')
						.Replace(':', '.')
						.Replace("`", "");
		}

		public static string XmlReaderToString(XmlReader reader)
		{
			string contents = "";
			XmlReader r = reader.ReadSubtree();
			while (r.Read())
			{
				if (r.NodeType == XmlNodeType.Element) {
					// TODO: This doesn't work, but it also doesn't crash
					if (r.Name == "see" || r.Name == "seealso")
					{
						contents += $"`{TypeName(r.GetAttribute("cref"), false)}`";
					}
					continue;
				}
				contents += r.ReadContentAsString();
			}
			contents = contents.Trim();
			return CleanMultiLine(contents);
		}

		public static string CleanForTable(string text)
		{
			return text.Replace('\n', ' ')
						.Replace('\r', ' ');
		}

		public static string CleanMultiLine(string text)
		{
			return string.Join("\n", text
				.Split('\n')
				.Select(a=>a.Trim())
				.ToArray());
		}

		public static string TypeName(string type, bool embedLink = true)
		{
			switch(type)
			{
				case "Single" : return "float";
				case "Double" : return "double";
				case "Int32"  : return "int";
				case "UInt32" : return "uint";
				case "String" : return "string";
				case "Boolean": return "bool";
				case "Void"   : return "void";
				default: {
					return embedLink && Program.TryGetClass(type, out DocClass typeDoc)
						? $"[{type}]({typeDoc.UrlName})"
						: type;
				}
			}
		}

		public static List<string> SeparateGroupedString(char separator, string str)
		{
			List<string> result = new List<string>();
			string curr = "";

			int bracketCt = 0;
			int parenCt = 0;
			int braceCt = 0;
			for (int i = 0; i < str.Length; i++)
			{
				if      (str[i] == '{') braceCt++;
				else if (str[i] == '[') bracketCt++;
				else if (str[i] == '(') parenCt++;
				else if (str[i] == '}') braceCt--;
				else if (str[i] == ']') bracketCt--;
				else if (str[i] == ')') parenCt--;
				if (braceCt == 0 && parenCt == 0 && bracketCt == 0 && str[i] == separator) { 
					result.Add(curr);
					curr = "";
				}
				else curr += str[i];
			}
			result.Add( curr );
			return result;
		}
	}
}
