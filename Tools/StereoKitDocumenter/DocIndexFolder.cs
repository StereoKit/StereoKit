using System.Collections.Generic;
using System.Text;

namespace StereoKitDocumenter
{
	class DocIndexFolder
	{
		public string name;
		public int    order;
		public List<DocIndexFolder> folders = new List<DocIndexFolder>();

		public DocIndexFolder(string aName, int aOrder = 0)
		{
			name  = aName;
			order = aOrder;
		}

		public override string ToString()
		{
			string result = "var documents =\n";
			result += ToString(this, 1);
			result = result.Substring(0, result.Length-2);
			result += ';';
			return result;
		}
		static string ToString(DocIndexFolder curr, int indent)
		{
			StringBuilder result = new StringBuilder();
			string ind = "";
			for (int i = 0; i < indent; i++) ind += '\t';

			result.Append($"{ind}{{name : '{curr.name}'");

			if (curr.folders.Count > 0) { 
				result.Append( $",\n{ind} pages: [\n");
				for (int i = 0; i < curr.folders.Count; i++)
				{
					result.Append(ToString(curr.folders[i], indent + 1));
				}
				result.Append($"{ind}]\n{ind}");
			}
			result.Append($"}},\n");

			return result.ToString();
		}
	}
}
