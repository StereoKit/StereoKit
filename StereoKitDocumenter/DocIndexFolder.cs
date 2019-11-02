using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitDocumenter
{
    class DocIndexFolder
    {
        public string name;
        public List<string> pages = new List<string>();
        public List<DocIndexFolder> folders = new List<DocIndexFolder>();

        public DocIndexFolder(string aName)
        {
            name = aName;
        }

        public override string ToString()
        {
            string result = "var documents =\n";
            result += ToString(this, 1);
            result = result.Substring(0, result.Length-2);
            result += ';';
            return result;
        }
        static string ToString(DocIndexFolder curr, int indent)
        {
            string result = "";
            string ind = "";
            for (int i = 0; i < indent; i++) ind += '\t';

            result += $"{ind}{{\n";
            result += $"{ind}name : '{curr.name}',\n";
            if (curr.pages.Count > 0)
                result += $"{ind}pages : ['{string.Join("','",curr.pages)}'],\n";
            else
                result += $"{ind}pages : [],\n";


            if (curr.folders.Count > 0) { 
                result += $"{ind}folders : [\n";
                for (int i = 0; i < curr.folders.Count; i++)
                {
                    result += ToString(curr.folders[i], indent + 1);
                }
                result += $"{ind}]\n";
            }
            result += $"{ind}}},\n";

            return result;
        }
    }
}
