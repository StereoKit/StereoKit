using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitDocumenter
{
    interface IDocItem
    {
        string Name { get; }
        string FileName { get; }
        string UrlName { get; }

        void AddExample(DocExample aExample);
    }
}
