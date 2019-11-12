using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using StereoKit;

namespace StereoKitDocumenter
{
    class DocMethod : IDocItem
    {
        public string   name;
        public DocClass parent;
        public List<DocExample>        examples  = new List<DocExample>();
        public List<DocMethodOverload> overloads = new List<DocMethodOverload>();

        public DocMethod(DocClass aParent, string aName)
        {
            parent = aParent;
            name   = aName;
            parent.methods.Add(this);
        }

        public DocMethodOverload AddOverload(string aSignature)
        {
            DocMethodOverload result = new DocMethodOverload(this, aSignature);
            overloads.Add(result);
            return result;
        }

        public string ShowName => name == "#ctor" ? parent.name : name;
        public string Name { get { return $"{parent.name}.{ShowName}"; } }
        public string FileName { get{ 
            return Path.Combine( Program.referenceOut, parent.name+"/"+ShowName+".md");
        } }
        public string UrlName { get{ 
            return $"{{{{site.url}}}}/Pages/Reference/{parent.name}/{ShowName}.html";
        } }

        public void AddExample(DocExample aExample) { examples.Add(aExample); }

        public override string ToString()
        {
            //if (name=="#ctor")
            //    return "---\nlayout: default\n---\n# Constructors not implmented yet.\n";

            string exampleText = "";
            if (examples.Count > 0) {
                exampleText = "\n\n## Examples\n\n";
                for (int i = 0; i < examples.Count; i++) {
                    exampleText += examples[i].data;
                }
            }

            return $@"---
layout: default
title: {parent.name}.{ShowName}
description: {StringHelper.CleanForDescription(overloads[0].summary)}
---
# [{parent.name}]({parent.UrlName}).{ShowName}

{string.Join("",overloads.Select(a=>a.ToString()).ToArray())}

{exampleText}
";

        }

        
    }
}
