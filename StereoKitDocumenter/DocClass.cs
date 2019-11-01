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
        public override string ToString()
        {

            return $@"
---
layout: default
title: {name}
description: {summary}
---
** {name}
___
{summary}
";

        }
    }
}
