using System.Collections.Generic;
using System.IO;

namespace StereoKitDocumenter
{
    class DocMethod
    {
        public string name;
        public string className;
        public string summary;
        public string returns;
        public List<DocParam> parameters = new List<DocParam>();

        public string FileName { get{ 
            return Path.Combine( Program.referenceOut, className+"/"+name+".md");
        } }
        public string UrlName { get{ 
            return $"/assets/pages/Reference/{className}/{name}.md";
        } }

        public override string ToString()
        {
            DocClass parent = Program.GetClass(className);

            string paramText = "";
            if (parameters.Count > 0) {
                paramText += "\n## Parameters\n|  |  |\n|--|--|\n";
                for (int i = 0; i < parameters.Count; i++) {
                    paramText += $"|{parameters[i].name}|{parameters[i].summary}|\n";
                }
            }

            return $@"
---
layout: default
title: {className}.{name}
description: {summary}
---
# [{className}]({parent.UrlName}).{name}
{paramText}
## Returns
{returns}

## Description
{summary}
";

        }
    }
}
