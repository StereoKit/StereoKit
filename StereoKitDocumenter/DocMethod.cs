using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitDocumenter
{
    class DocMethod
    {
        public string name;
        public string className;
        public string summary;
        public string returns;

        public string FileName { get{ 
            return Path.Combine( Program.referenceOut, name+".md");
        } }

        public override string ToString()
        {
            
            return $@"
---
layout: default
title: {className}.{name}
description: {summary}
---
** {className}.{name}
___
{summary}

{returns}
";

        }
    }
}
