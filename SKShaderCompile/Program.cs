using System;
using System.IO;
using System.Text;
using StereoKit;

enum FileFormat {
    Binary,
    C
}

class Program
{
    static void Main(string[] args)
    {
        string path   = "";
        string search = "";
        string output = "";
        FileFormat format = FileFormat.Binary;
        for (int i = 0; i < args.Length; i++)
        {
            if (i+1 < args.Length) { 
                if (args[i].ToLower() == "-path")
                {
                    path   = Path.GetDirectoryName(args[i+1]);
                    search = Path.GetFileName     (args[i+1]);
                    i++;
                    continue;
                }
                else if (args[i].ToLower() == "-out")
                {
                    output = args[i+1];
                    i++;
                    continue;
                }
            }
            if (args[i].ToLower() == "-c")
            {
                format = FileFormat.C;
                continue;
            }
        }
        if (string.IsNullOrEmpty(output))
            output = path;

        string[] files = Directory.GetFiles(path, search);

        for (int i = 0; i < files.Length; i++)
        {
            string text   = File.ReadAllText(files[i]);
            byte[] shader = Shader.Compile(text);

            if (shader.Length == 0)
            {
                Console.WriteLine("Error compiling shader file: " + files[i]);
                return;
            }

            string fileName = Path.GetFileNameWithoutExtension(files[i]);
            string fullFile = Path.Combine(output, fileName + ".sks");
            Save(fullFile, shader, format);
            Console.WriteLine("Compiled: " + fileName);
        }
    }

    static void Save(string filename, byte[] bytes, FileFormat format)
    {
        if (format == FileFormat.Binary)
            File.WriteAllBytes(filename, bytes);
        else if (format == FileFormat.C)
        {
            string name = Path.GetFileNameWithoutExtension(filename);
            StringBuilder result = new StringBuilder();
            result.Append($"#include \"{name}.h\"\n\n");
            string start = $"const char {name}[{bytes.Length}] = {{";
            result.Append(start);
            int length = start.Length;
            for (int i = 0; i < bytes.Length; i++)
            {
                // I started with hex representation, but this is actually
                // much smaller. 0x00 is 4 chars, where 255 is 3, and 0 is 
                // just 1 in the best case scenario.
                result.Append($"{bytes[i]},");

                if      (bytes[i] < 10 ) length += 2;
                else if (bytes[i] < 100) length += 3;
                else                     length += 4;

                if (length > 75)
                {
                    result.Append('\n');
                    length = 0;
                }
            }
            result.Append("};\n");
            File.WriteAllText(Path.ChangeExtension(filename,"h"), $"#pragma once\n\nextern const char {name}[{bytes.Length}];\n");
            File.WriteAllText(Path.ChangeExtension(filename,"cpp"), result.ToString());
        }
    }
}