using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitDocumenter
{
    class DocClass : IDocItem
    {
        public string name;
        public string summary;
        public List<DocMethod> methods = new List<DocMethod>();
        public List<DocField> fields = new List<DocField>();
        public List<DocExample> examples = new List<DocExample>();

        public string Name { get { return name; } }
        public string FileName { get{ 
            return Path.Combine( Program.referenceOut, name+".md");
        } }
        public string UrlName { get{ 
            return $"{{{{site.url}}}}/Pages/Reference/{name}.html";
        } }

        public void AddExample(DocExample aExample) { examples.Add(aExample); }

        public override string ToString()
        {
            Type t = Type.GetType("StereoKit." + name + ", StereoKit");
            methods.Sort((a,b)=>a.name.CompareTo(b.name));
            fields .Sort((a,b)=>a.name.CompareTo(b.name));

            string memberText = "";
            if (methods.Count > 0) {
                memberText = "\n\n## Methods\n\n|  |  |\n|--|--|\n";
                for (int i = 0; i < methods.Count; i++)
                {
                    memberText += $"|[{methods[i].ShowName}]({methods[i].UrlName})|{StringHelper.CleanForTable(methods[i].overloads[0].summary)}|\n";
                }
            }

            string fieldText = "";
            if (fields.Count > 0) { 
                fieldText = "\n\n## Fields and Properties\n\n|  |  |\n|--|--|\n";
                for (int i = 0; i < fields.Count; i++)
                {
                    fieldText += $"|{StringHelper.TypeName(fields[i].GetFieldType(t).Name)} [{fields[i].name}]({fields[i].UrlName})|{StringHelper.CleanForTable(fields[i].summary)}|\n";
                }
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
title: {name}
description: {StringHelper.CleanForDescription(summary)}
---
# {name}

{summary}
{fieldText}
{memberText}
{exampleText}
";

        }
    }
}
