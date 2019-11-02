using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitDocumenter
{
    class DocClass
    {
        public string name;
        public string summary;

        public string FileName { get{ 
            return Path.Combine( Program.referenceOut, name+".md");
        } }
        public string UrlName { get{ 
            return $"/assets/pages/Reference/{name}.md";
        } }

        public override string ToString()
        {
            
            List<DocMethod> methods = new List<DocMethod>();
            for (int i = 0; i < Program.methods.Count; i++)
            {
                if (Program.methods[i].className == name) 
                    methods.Add(Program.methods[i]);
            }
            methods.Sort((a,b)=>a.name.CompareTo(b.name));

            string memberText = "";
            for (int i = 0; i < methods.Count; i++)
            {
                memberText += $"|[{methods[i].name}]({methods[i].UrlName})|{methods[i].summary}|\n";
            }

            return $@"
---
layout: default
title: {name}
description: {summary}
---
# {name}

## Description
{summary}

## Methods
|  |  |
|--|--|
{memberText}
";

        }
    }
}
