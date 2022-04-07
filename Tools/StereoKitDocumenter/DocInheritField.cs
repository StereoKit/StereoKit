using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitDocumenter
{
	class DocInheritField
	{
		DocField destinationField;
		string   inheritReference;

		public DocInheritField(DocField destination, string reference)
		{
			destinationField = destination;
			inheritReference = reference.Trim();
			int start = inheritReference.IndexOf('.');
			start = start == -1 ? 0 : start + 1;
			inheritReference = inheritReference.Substring(start);
		}

		public void Resolve(List<DocField> fields)
		{
			string[] words = inheritReference.Split('.');
			if (words.Length != 2)
			{
				Console.WriteLine($"Inheritdoc on {destinationField.Name} has more components than understood: '{inheritReference}'");
				return;
			}

			for (int i = 0; i < fields.Count; i++)
			{
				if (destinationField == fields[i]) continue;

				if (fields[i].parent.name == words[0] &&
					fields[i].name        == words[1])
				{
					destinationField.summary = fields[i].summary;
					return;
				}
			}
			Console.WriteLine($"Unresolved inheritdoc for {destinationField.Name}, reference not found: '{inheritReference}'");
		}
	}
}
