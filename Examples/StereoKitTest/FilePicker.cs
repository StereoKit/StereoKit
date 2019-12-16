using StereoKit;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitTest
{
    class FilePicker
    {
        #region Support types
        public enum FileType
        {
            File,
            Folder
        }
        public struct File
        {
            public FileType type;
            public string   name;
            public string   extension;
        }
        #endregion

        #region Fields
        (string,string)[] filters;
        List<File>        activeFiles = new List<File>();
        int               selectedIndex = -1;
        string            path;
        string            title;
        string[]          separatedPath;
        Pose              windowPose = new Pose(new Vec3(-10,0,0)*Units.cm2m, Quat.LookDir(-Vec3.Forward));
        #endregion

        public string SelectedFile { 
            get => selectedIndex == -1 ? 
                "" : 
                Path.Combine(path, activeFiles[selectedIndex].name + activeFiles[selectedIndex].extension);
        }

        public FilePicker((string name, string filter)[] filters, string initialFolder=null)
        {
            foreach (var s in filters)
                this.title += s.name+"/";
            this.filters = filters;
            UpdateFolder(initialFolder == null ? Directory.GetCurrentDirectory() : initialFolder);
        }

        bool Select(int index)
        {
            selectedIndex = index;
            if (activeFiles[index].type == FileType.Folder)
            {
                string newPath = Path.Combine(path, activeFiles[index].name);
                UpdateFolder(newPath);
            } 
            else
            {
                return true;
            }
            return false;
        }

        void UpdateFolder(string newPath)
        {
            path = newPath;

            // Find the files and folders at the current path!
            List<string> files = new List<string>( filters
                .SelectMany(f => Directory.GetFiles(path, f.Item2))
                .OrderBy   (f => f));
            string[] folders = Directory.GetDirectories(path);

            // Parse files and folders into data we can draw!
            activeFiles.Clear();
            activeFiles.AddRange(files.Select(f => new File { 
                type      = FileType.File, 
                extension = Path.GetExtension(f),
                name      = Path.GetFileNameWithoutExtension(f)}));
            activeFiles.AddRange(folders.Select(f=>new File{ 
                type      = FileType.Folder, 
                extension = "",
                name      = Path.GetFileName(f)}));

            // Parse the path for display
            separatedPath = path.Split(new char[]{Path.DirectorySeparatorChar,Path.AltDirectorySeparatorChar}, StringSplitOptions.RemoveEmptyEntries);
            for (int i = 0; i < separatedPath.Length; i++) {
                separatedPath[i] += Path.DirectorySeparatorChar;
            }
        }

        public bool Show()
        {
            bool result = false;

            UI.WindowBegin(title, ref windowPose, new Vec2(40,0) *Units.cm2m);
            UI.Label("...\\");
            for (int i = Math.Max(0,separatedPath.Length-3); i < separatedPath.Length; i++) {
                UI.SameLine();
                if (UI.Button(separatedPath[i])) {
                    string newPath = "";
                    for (int t = 0; t <= i; t++)
                        newPath += separatedPath[t] + Path.DirectorySeparatorChar;
                    UpdateFolder(newPath);
                }
            }
            UI.Space(Units.cm2m * 1.5f);

            for (int i = 0; i < activeFiles.Count; i++)
            {
                if (UI.Button(activeFiles[i].name+"\n"+activeFiles[i].extension))
                    result = Select(i);
                UI.SameLine();
            }
            UI.WindowEnd();

            return result;
        }
    }
}
