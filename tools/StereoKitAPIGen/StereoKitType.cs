using CppAst;
using System.Collections.Generic;


static class StereoKitTypes
{
	public static HashSet<string> types;
	
	public static void Parse(CppCompilation ast)
	{
		types = new HashSet<string>();
		
		// Find all our custom asset types
		CppContainerList<CppTypedef> typedefs = ast.Namespaces[0].Typedefs;
		
		foreach (var t in typedefs)
		{
			if (t.ElementType is CppPointerType && ((CppPointerType)t.ElementType).GetDisplayName().StartsWith("_"))
			{
				types.Add(t.GetDisplayName());
			}
		}
	}
}
