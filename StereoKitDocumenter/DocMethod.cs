using System.Collections.Generic;
using System.IO;

namespace StereoKitDocumenter
{
    class DocMethod : IDocItem
    {
        public string name;
        public string summary;
        public string returns;
        public DocClass parent;
        public List<DocParam> parameters = new List<DocParam>();

        public DocMethod(DocClass aParent, string aName)
        {
            parent = aParent;
            name   = aName;
            parent.methods.Add(this);
        }

        public string FileName { get{ 
            return Path.Combine( Program.referenceOut, parent.name+"/"+name+".md");
        } }
        public string UrlName { get{ 
            return $"/assets/pages/Reference/{parent.name}/{name}.html";
        } }

        public override string ToString()
        {
            string paramText = "";
            if (parameters.Count > 0) {
                paramText += "\n## Parameters\n\n|  |  |\n|--|--|\n";
                for (int i = 0; i < parameters.Count; i++) {
                    paramText += $"|{parameters[i].name}|{parameters[i].summary}|\n";
                }
            }

            return $@"---
layout: default
title: {parent.name}.{name}
description: {summary}
---
# [{parent.name}]({parent.UrlName}).{name}
{paramText}
## Returns
{returns}

## Description
{summary}
";

        }
    }
}
