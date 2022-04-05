using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitDocumenter
{
	class DocInheritMethod
	{
		DocMethodOverload destinationMethod;
		string            inheritReference;

		public DocInheritMethod(DocMethodOverload destination, string reference)
		{
			destinationMethod = destination;
			inheritReference  = reference.Trim();
			int start = inheritReference.IndexOf('.');
			start = start == -1 ? 0 : start + 1;
			inheritReference = inheritReference.Substring(start);
		}

		public void Resolve(List<DocMethod> methods)
		{
			string[] segs = inheritReference.Split('(');
			string nameSignature  = segs[0];
			string paramSignature = segs.Length > 1 ? segs[1] : "";
			if (paramSignature.Length > 0)
				paramSignature = paramSignature.Substring(0, paramSignature.Length - 1);

			string[] names = nameSignature.Split('.');
			string[] mparams = paramSignature.Split(',');
			if (names.Length != 2)
			{
				Console.WriteLine($"Inheritdoc on {destinationMethod.signature} has more components than understood: '{inheritReference}'");
				return;
			}

			for (int i = 0; i < methods.Count; i++)
			{
				if (methods[i].parent.name != names[0] ||
					methods[i].name != names[1]) continue;

				for (int v = 0; v < methods[i].overloads.Count; v++)
				{
					DocMethodOverload ov = methods[i].overloads[v];
					if (ov == destinationMethod) continue;
					if (ov.signature == paramSignature)
					{
						if (string.IsNullOrEmpty(destinationMethod.summary))
							destinationMethod.summary = ov.summary;
						if (string.IsNullOrEmpty(destinationMethod.returns))
							destinationMethod.returns = ov.returns;

						destinationMethod.parameters.InsertRange(0, ov.parameters);
						return;
					}
				}
			}
			Console.WriteLine($"Unresolved inheritdoc for {destinationMethod.signature}, reference not found: '{inheritReference}'");
		}
	}
}
