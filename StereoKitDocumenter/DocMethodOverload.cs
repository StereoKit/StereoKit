using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitDocumenter
{
    class DocMethodOverload
    {
        public DocMethod rootMethod;
        public string signature;
        public string summary;
        public string returns;
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

            string paramList = string.Join(", ", param.Select(a => $"{StringHelper.TypeName(a.ParameterType.Name)} {a.Name}"));
            string signature = (m.IsStatic ? "static " : "") + $"{returnName} {methodName}({paramList})";

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
                        throw new Exception("Missing doc tag for the return value of " + methodName);
                    paramText += $"|RETURNS: {returnName}|{StringHelper.CleanForTable(returns)}|\n";
                }
            }

            return $@"<div class='signature' markdown='1'>
{signature}
</div>
{paramText}
{summary}
";
        }

        private static Type GetParentType(DocMethod rootMethod)
        {
            return Type.GetType("StereoKit." + rootMethod.parent.name + ", StereoKit");
        }
        private static MethodBase GetMethodInfo(string signature, DocMethod rootMethod)
        {
            Type[] paramTypes = string.IsNullOrEmpty(signature) ? new Type[]{ } : signature
                .Split(',')
                .AsEnumerable()
                .Select(a => {
                    string cleanName = a.Replace("@", "");
                    bool   nullable  = a.Contains("System.Nullable");
                    if (nullable)
                    {
                        int length = "System.Nullable{".Length;
                        cleanName = cleanName.Substring(length, cleanName.Length-length-1);
                    }

                    Type t = Type.GetType(cleanName);
                    if (t == null)
                        t = Type.GetType(cleanName + ", StereoKit");
                    if (a.Contains("@"))
                        t = t.MakeByRefType();
                    if (nullable)
                        t = typeof(Nullable<>).MakeGenericType(t);

                    return t;
                    })
                .ToArray();

            Type parent = GetParentType(rootMethod);
            MethodBase result = rootMethod.name == "#ctor" ?
                (MethodBase)parent.GetConstructor(paramTypes) :
                (MethodBase)parent.GetMethod     (rootMethod.name, paramTypes);

            if (result == null)
                throw new Exception("Can't find info for method " + rootMethod.name);
            return result;
        }
    }
}
