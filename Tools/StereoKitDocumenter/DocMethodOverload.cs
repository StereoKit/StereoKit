using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace StereoKitDocumenter
{
	struct GenericStructT { }
	class GenericClassT { }

	class DocMethodOverload
	{
		public DocMethod      rootMethod;
		public string         signature;
		public string         summary;
		public string         returns;
		public List<DocParam> parameters = new List<DocParam>();

		MethodBase methodInfo;

		public bool IsStatic => methodInfo.IsStatic;

		public DocMethodOverload(DocMethod aRootMethod, string aSignature)
		{
			rootMethod = aRootMethod;
			signature  = aSignature;
			methodInfo = GetMethodInfo(signature, rootMethod);
		}

		public override string ToString()
		{
			MethodBase m = methodInfo;
			Type   returnType = m is MethodInfo ? ((MethodInfo)m).ReturnType : typeof(void);
			string methodName = rootMethod.ShowName;
			string returnName = m is MethodInfo ? StringHelper.TypeName(returnType.Name) : "";
			List<ParameterInfo> param = m == null ? new List<ParameterInfo>() : new List<ParameterInfo>(m.GetParameters());

			string paramList = string.Join(", ", param.Select(a => $"{StringHelper.TypeName(a.ParameterType.Name, false)} {a.Name}"));
			string signature = (m.IsStatic ? "static " : "") + $"{StringHelper.TypeName(returnType.Name, false)} {methodName}({paramList})";

			string paramText = "";
			if (parameters.Count > 0 || returnType != typeof(void))
			{
				paramText += "\n|  |  |\n|--|--|\n";
				for (int i = 0; i < parameters.Count; i++)
				{
					ParameterInfo p = param.Find(a => a.Name == parameters[i].name);
					if (p == null)
						throw new Exception($"Can't find document paramter {parameters[i].name} in {rootMethod.name}");
					paramText += $"|{StringHelper.TypeName(p.ParameterType.Name)} {parameters[i].name}|{StringHelper.CleanForTable(parameters[i].summary)}|\n";
				}

				if (returnType != typeof(void)) {
					if (string.IsNullOrEmpty( returns ))
						throw new Exception("Missing doc tag for the return value of " + rootMethod.Name);
					paramText += $"|RETURNS: {returnName}|{StringHelper.CleanForTable(returns)}|\n";
				}
			}

			return $@"<div class='signature' markdown='1'>
```csharp
{signature}
```
{summary}
</div>
{paramText}
";
		}

		private static Type GetParentType(DocMethod rootMethod)
		{
			Type result = Type.GetType("StereoKit." + rootMethod.parent.name + ", StereoKit");
			if (result == null)
				result = Type.GetType("StereoKit.Framework." + rootMethod.parent.name + ", StereoKit");
			if (result == null)
				result = Type.GetType("StereoKit.Backend+" + rootMethod.parent.name + ", StereoKit");
			return result;
		}
		private static MethodBase GetMethodInfo(string signature, DocMethod rootMethod)
		{
			Type[] paramTypes = string.IsNullOrEmpty(signature) ? new Type[]{ } : StringHelper.SeparateGroupedString(',',signature)
				.Select(a => {
					string cleanName = a.Replace("@", "");
					bool   nullable  = a.Contains("System.Nullable");
					bool   action    = a.Contains("System.Action{");
					bool   array     = a.Contains("[]");
					bool   generic   = a.Contains("`");
					if (nullable)
					{
						int length = "System.Nullable{".Length;
						cleanName = cleanName.Substring(length, cleanName.Length-length-1);
					}
					if (action)
					{
						int length = "System.Action{".Length;
						cleanName = cleanName.Substring(length, cleanName.Length - length - 1);
					}
					if (array)
					{
						cleanName = cleanName.Substring(0, cleanName.IndexOf("[]"));
					}

					int commas = cleanName.Count(c => c == ',');
					Type t = null;
					if (t == null && action && commas == 0)
						t = typeof(Action<>).MakeGenericType(Type.GetType(cleanName));
					if (t == null && action && commas == 1)
						t = typeof(Action<,>).MakeGenericType(cleanName.Split(',').Select(n => Type.GetType(n)).ToArray());
					if (t == null && action && commas == 2)
						t = typeof(Action<,,>).MakeGenericType(cleanName.Split(',').Select(n => Type.GetType(n)).ToArray());
					if (t == null) 
						t = Type.GetType(cleanName);
					if (t == null)
						t = Type.GetType(cleanName + ", StereoKit");
					if (t == null)
						t = Type.GetType(cleanName + ", " + typeof(System.Numerics.Vector3).Assembly.FullName);
					if (t != null && nullable)
						t = typeof(Nullable<>).MakeGenericType(t);
					if (t != null && array)
						t = t.MakeArrayType();
					if (t != null && a.Contains("@"))
						t = t.MakeByRefType();
					if (t == null && generic)
						t = typeof(object);

					if (t == null)
						throw new Exception($"Can't find {rootMethod.Name}'s parameter type: {a}!");
					return t;
				})
				.ToArray();

			// Scrape out generics tags, don't quite know how to use them yet.
			string methodName = rootMethod.name;
			if (methodName.Contains('`'))
				methodName = methodName.Substring(0, methodName.IndexOf('`'));

			Type       parent = GetParentType(rootMethod);
			MethodBase result = methodName == "#ctor" ?
				(MethodBase)parent.GetConstructor(paramTypes) :
				(MethodBase)parent.GetMethod     (methodName, paramTypes);

			// If it's generic, but there's no overloads, we can just return
			// the only method present
			if (result == null && methodName != "#ctor" && paramTypes.Contains(typeof(object)) && parent.GetMethods().Where(m=>m.Name==methodName).Count() == 1)
				result = parent.GetMethod(methodName);

			if (result == null)
				throw new Exception("Can't find info for method " + rootMethod.name);
			return result;
		}
	}
}
