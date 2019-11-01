using System;
using System.Xml;

namespace StereoKitDocumenter
{
    class Program
    {
        static void Main(string[] args)
        {
            string xmlDocs = "../../../bin/StereoKit.xml";

            XmlReader reader = XmlReader.Create(xmlDocs);
            while(reader.ReadToFollowing("member"))
            {
                string name = reader.GetAttribute("name");
                XmlReader member = reader.ReadSubtree();

                XmlReader summary = member;
                summary.ReadToFollowing("summary");
                
                Console.WriteLine(name);
                Console.WriteLine(summary.ReadElementContentAsString().Trim());
            }
        }
    }
}
