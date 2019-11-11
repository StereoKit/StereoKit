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

        public DocMethodOverload(DocMethod aRootMethod, string aSignature)
        {
            rootMethod = aRootMethod;
            signature = aSignature;
        }

        public override string ToString()
        {
            if (rootMethod.name == "#ctor")
                return "---\nlayout: default\n---\n# Constructors not implmented yet.\n";

            MethodInfo m = GetMethodInfo();
            List<ParameterInfo> param = m == null ? new List<ParameterInfo>() : new List<ParameterInfo>(m.GetParameters());

            string paramList = string.Join(", ", param.Select(a => $"{StringHelper.TypeName(a.ParameterType.Name)} {a.Name}"));
            string signature = (m.IsStatic ? "static " : "") + $"{StringHelper.TypeName(m.ReturnType.Name)} {m.Name}({paramList})";

            string paramText = "";
            if (parameters.Count > 0 || m.ReturnType != typeof(void))
            {
                paramText += "\n|  |  |\n|--|--|\n";
                for (int i = 0; i < parameters.Count; i++)
                {

                    ParameterInfo p = param.Find(a => a.Name == parameters[i].name);
                    paramText += $"|{StringHelper.TypeName(p.ParameterType.Name)} {parameters[i].name}|{StringHelper.CleanForTable(parameters[i].summary)}|\n";
                }

                if (m.ReturnType != typeof(void))
                    paramText += $"|RETURNS: {StringHelper.TypeName(m.ReturnType.Name)}|{StringHelper.CleanForTable(returns)}|\n";
            }

            string exampleText = "";
            if (examples.Count > 0)
            {
                exampleText = "\n\n## Examples\n\n";
                for (int i = 0; i < examples.Count; i++)
                {
                    exampleText += examples[i].data;
                }
            }

            return $@"---
layout: default
title: {parent.name}.{name}
description: {StringHelper.CleanForDescription(summary)}
---
# [{parent.name}]({parent.UrlName}).{name}
<div class='signature' markdown='1'>
{signature}
</div>
{summary}
{paramText}

{exampleText}
";
        }

        private Type GetParentType()
        {
            return Type.GetType("StereoKit." + parent.name + ", StereoKit");
        }
        private MethodInfo GetMethodInfo()
        {
            Type[] paramTypes = signature
                .Split(',')
                .AsEnumerable()
                .Select(a => Type.GetType(a))
                .ToArray();

            Type parent = GetParentType();
            MethodInfo result = parent.GetMethod(name, paramTypes);
            return result;
        }
    }
}
