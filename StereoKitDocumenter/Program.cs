using System;
using System.Collections.Generic;
using System.IO;
using System.Xml;

namespace StereoKitDocumenter
{
    class Program
    {
        public const string xmlDocs = "../../../bin/StereoKit.xml";
        public const string referenceOut = "../../../docs/assets/pages/reference/";

        static List<DocClass>  classes = new List<DocClass>();
        static List<DocMethod> methods = new List<DocMethod>();

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
            DocMethod result = new DocMethod();

            // Get names
            string[] segs = signature.Split('(');
            string nameSignature  = segs[0];
            string paramSignature = segs.Length>1?segs[1]:"";
            segs = nameSignature.Split('.');
            if (segs.Length != 3)
                Console.WriteLine("Unexpected signature length, " + signature);
            result.name      = segs[2];
            result.className = segs[1];

            // Read properties
            while (reader.Read())
            {
                switch(reader.Name.ToLower())
                {
                    case "summary": result.summary = reader.ReadElementContentAsString().Trim(); break;
                    case "returns": result.returns = reader.ReadElementContentAsString().Trim(); break;
                }
            }

            methods.Add(result);
        }
    }
}
