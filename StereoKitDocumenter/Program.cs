using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml;

namespace StereoKitDocumenter
{
    class Program
    {
        public const string xmlDocs = "../../../bin/StereoKit.xml";
        public const string pagesOut = "../../../docs/assets/pages/";
        public const string referenceOut = pagesOut+"reference/";

        public static List<DocClass>  classes = new List<DocClass>();
        public static List<DocMethod> methods = new List<DocMethod>();

        public static DocClass GetClass(string name) { return classes.Find((a)=>a.name==name); }

        static void Main(string[] args)
        {
            XmlReader reader = XmlReader.Create(xmlDocs);
            while(reader.ReadToFollowing("member"))
            {
                string    name = reader.GetAttribute("name");
                XmlReader member = reader.ReadSubtree();
                string    type = name[0].ToString();
                string    signature = name.Substring(2);

                if (type == "T") {
                    ReadClass(signature, reader.ReadSubtree());
                } else if (type == "M") {
                    ReadMethod(signature, reader.ReadSubtree());
                }
            }

            for (int i = 0; i < classes.Count; i++)
            {
                StreamWriter writer =  new StreamWriter(classes[i].FileName);
                writer.Write(classes[i].ToString());
                writer.Close();
            }
            for (int i = 0; i < methods.Count; i++)
            {
                StreamWriter writer = new StreamWriter(methods[i].FileName);
                writer.Write(methods[i].ToString());
                writer.Close();
            }

            { 
                StreamWriter writer = new StreamWriter(pagesOut+"data.js");
                writer.Write(WriteIndex());
                writer.Close();
            }
        }

        static void ReadClass(string signature, XmlReader reader)
        {
            DocClass result = new DocClass();

            // Get names
            string[] segs = signature.Split('.');
            if (segs.Length != 2)
                Console.WriteLine("Unexpected signature length, " + signature);
            result.name = segs[1];

            // Read properties
            while (reader.Read())
            {
                switch (reader.Name.ToLower())
                {
                    case "summary": result.summary = reader.ReadElementContentAsString().Trim(); break;
                }
            }

            classes.Add(result);
        }

        static void ReadMethod(string signature, XmlReader reader)
        {
            // Get names
            string[] segs = signature.Split('(');
            string nameSignature  = segs[0];
            string paramSignature = segs.Length>1?segs[1]:"";
            segs = nameSignature.Split('.');
            if (segs.Length != 3)
                Console.WriteLine("Unexpected signature length, " + signature);

            DocMethod result = new DocMethod(GetClass(segs[1]), segs[2]);

            // Read properties
            while (reader.Read())
            {
                switch(reader.Name.ToLower())
                {
                    case "summary": result.summary = reader.ReadElementContentAsString().Trim(); break;
                    case "returns": result.returns = reader.ReadElementContentAsString().Trim(); break;
                    case "param": {
                        DocParam p = new DocParam();
                        p.name    = reader.GetAttribute("name");
                        p.summary = reader.ReadElementContentAsString().Trim();
                        result.parameters.Add(p);
                    } break;
                }
            }

            methods.Add(result);
        }

        static string WriteIndex()
        {
            DocIndexFolder root = new DocIndexFolder("pages");
            DocIndexFolder reference = new DocIndexFolder("Reference");
            root.folders.Add(reference);

            for (int i = 0; i < classes.Count; i++)
            {
                DocIndexFolder classFolder = new DocIndexFolder(classes[i].name);
                reference.folders.Add(classFolder);

                for (int m = 0; m < classes[i].methods.Count; m++)
                {
                    classFolder.pages.Add(classes[i].methods[m].name);
                }

            }

            return root.ToString();
        }
    }
}
