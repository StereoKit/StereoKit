using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitDocumenter
{
    class DocField : IDocItem
    {
        public string name;
        public string summary;
        public DocClass parent;
        public List<DocExample> examples = new List<DocExample>();

        public DocField(DocClass aParent, string aName)
        {
            parent = aParent;
            name   = aName;
            parent.fields.Add(this);
        }

        public string Name { get { return $"{parent.name}.{name}"; } }
        public string FileName { get {
            return Path.Combine(Program.referenceOut, parent.name + "/" + name + ".md");
        } }
        public string UrlName { get {
            return $"/assets/pages/Reference/{parent.name}/{name}.html";
        } }

        public void AddExample(DocExample aExample) { examples.Add(aExample); }

        public override string ToString()
        {
            string exampleText = "";
            if (examples.Count > 0) {
                exampleText = "\n\n## Examples\n\n";
                for (int i = 0; i < examples.Count; i++) {
                    exampleText += examples[i];
                }
            }
            return $@"---
layout: default
title: {parent.name}.{name}
description: {StringHelper.CleanForDescription(summary)}
---
# [{parent.name}]({parent.UrlName}).{name}

## Description
{summary}
{exampleText}
";

        }
    }
}
