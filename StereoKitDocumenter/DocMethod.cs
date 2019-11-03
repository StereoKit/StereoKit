using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using StereoKit;

namespace StereoKitDocumenter
{
    class DocMethod : IDocItem
    {
        public string name;
        public string summary;
        public string returns;
        public DocClass parent;
        public List<DocParam> parameters = new List<DocParam>();
        public List<DocExample> examples = new List<DocExample>();

        public DocMethod(DocClass aParent, string aName)
        {
            parent = aParent;
            name   = aName;
            parent.methods.Add(this);
        }

        public string Name { get { return $"{parent.name}.{name}"; } }
        public string FileName { get{ 
            return Path.Combine( Program.referenceOut, parent.name+"/"+name+".md");
        } }
        public string UrlName { get{ 
            return $"{{{{site.url}}}}/Pages/Reference/{parent.name}/{name}.html";
        } }

        public void AddExample(DocExample aExample) { examples.Add(aExample); }

        public override string ToString()
        {
            Vec3 vec;
            Type t = Type.GetType("StereoKit."+parent.name+", StereoKit");
            MethodInfo m = t.GetMethod(name);
            List<ParameterInfo> param = new List<ParameterInfo>(m.GetParameters());

            string paramList = string.Join(", ", param.Select(a=> $"{StringHelper.TypeName(a.ParameterType.Name)} {a.Name}" ));
            string signature = (m.IsStatic ? "static " : "")+$"{StringHelper.TypeName(m.ReturnType.Name)} {m.Name}({paramList})";

            string paramText = "";
            if (parameters.Count > 0 || m.ReturnType != typeof(void)) {
                paramText += "\n## Parameters\n\n|  |  |\n|--|--|\n";
                for (int i = 0; i < parameters.Count; i++) {
                    
                    ParameterInfo p = param.Find(a => a.Name == parameters[i].name);
                    paramText += $"|{StringHelper.TypeName(p.ParameterType.Name)} {parameters[i].name}|{StringHelper.CleanForTable(parameters[i].summary)}|\n";
                }

                if (m.ReturnType != typeof(void))
                    paramText += $"|RETURNS: {StringHelper.TypeName(m.ReturnType.Name)}|{StringHelper.CleanForTable(returns)}|\n";
            }

            string exampleText = "";
            if (examples.Count > 0) {
                exampleText = "\n\n## Examples\n\n";
                for (int i = 0; i < examples.Count; i++) {
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
{paramText}

## Description
{summary}
{exampleText}
";

        }
    }
}
