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
            List<DocMethod> methodsStatic   = methods.FindAll(a =>  a.IsStatic);
            List<DocMethod> methodsInstance = methods.FindAll(a => !a.IsStatic);
            List<DocField>  fieldsStatic    = fields.FindAll(a =>  a.GetStatic(t));
            List<DocField>  fieldsInstance  = fields.FindAll(a => !a.GetStatic(t));
            
            Func<DocField,  string> fieldToString  = (f) => { return $"|{StringHelper.TypeName(f.GetFieldType(t).Name)} [{f.name}]({f.UrlName})|{StringHelper.CleanForTable(f.summary)}|"; };
            Func<DocMethod, string> methodToString = (m) => { return $"|[{m.ShowName}]({m.UrlName})|{StringHelper.CleanForTable(m.overloads[0].summary)}|"; };
            
            string memberText = methodsInstance.Count == 0 ? 
                "" : "\n\n## Instance Methods\n\n|  |  |\n|--|--|\n";
            memberText += string.Join("\n", methodsInstance
                .Select(methodToString));

            string fieldText = fieldsInstance.Count == 0 ? 
                "" : "\n\n## Instance Fields and Properties\n\n|  |  |\n|--|--|\n";
            fieldText += string.Join("\n", fieldsInstance
                .Select(fieldToString));

            string memberTextStatic = methodsStatic.Count == 0 ?
                "" : "\n\n## Static Methods\n\n|  |  |\n|--|--|\n";
            memberTextStatic += string.Join("\n", methodsStatic
                .Select(methodToString));

            string fieldTextStatic = fieldsStatic.Count == 0 ?
                "" : "\n\n## Static Fields and Properties\n\n|  |  |\n|--|--|\n";
            fieldTextStatic += string.Join("\n", fieldsStatic
                .Select(fieldToString));

            string exampleText = examples.Count == 0 ?
                "" : "\n\n## Examples\n\n";
            exampleText += string.Join("\n", examples.Select(e=>e.data));

            return $@"---
layout: default
title: {name}
description: {StringHelper.CleanForDescription(summary)}
---
# {name}

{summary}
{fieldText}
{memberText}
{fieldTextStatic}
{memberTextStatic}
{exampleText}
";

        }
    }
}
