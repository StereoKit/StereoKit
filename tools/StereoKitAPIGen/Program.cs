using CppAst;
using System;
using System.Collections.Generic;

class Program
{
	enum BindLang
	{
		None,
		CSharp,
	}

	static void Main(string[] args)
	{
		// This application needs parameters to work!
		if (args.Length == 0)
		{
			//-f "$(SolutionDir)\StereoKitC\stereokit.h" -o SKOverrides.txt -d "$(SolutionDir)\StereoKit\Native"
			args = new string[] { 
				"-f", "../../../../../StereoKitC/stereokit.h",
				"-o", "SKOverrides.txt",
				"-d", "../../../../../StereoKit/Native"};
		}

		// Parse the parameters into stuff we can work with
		List<string> files      = new List<string>();
		BindLang     lang       = BindLang.None;
		string       destFolder = "";
		int          curr       = 0;
		while (curr < args.Length)
		{
			if (args[curr].ToLower() == "-o")
			{
				if (curr + 1 < args.Length) {
					if (!NameOverrides.Load(args[curr + 1])) return;
					curr += 2;
				} else { 
					Console.WriteLine("ERROR: Missing parameter for -o, need a file path for name overrides!");
					DisplayHelp();
					return;
				}
			}
			else if (args[curr].ToLower() == "-f")
			{
				if (curr + 1 < args.Length) {
					files.Add(args[curr + 1]);
					curr += 2;
				} else { 
					Console.WriteLine("ERROR: Missing parameter for -f, need a file path to a C++ header!");
					DisplayHelp();
					return;
				}
			}
			else if (args[curr].ToLower() == "-l")
			{
				if (curr + 1 < args.Length) {
					if (Enum.TryParse(args[curr + 1], true, out BindLang langArg)) lang = langArg;
					else { Console.WriteLine($"Invalid language identifier '{args[curr + 1]}'!"); return; }

					curr += 2;
				} else { 
					Console.WriteLine("ERROR: Missing parameter for -l, need a language identifier!");
					DisplayHelp();
					return;
				}
			}
			else if (args[curr].ToLower() == "-d")
			{
				if (curr + 1 < args.Length) {
					destFolder = args[curr + 1];
					curr += 2;
				} else { 
					Console.WriteLine("ERROR: Missing parameter for -d, need a file path to a destination folder!");
					DisplayHelp();
					return;
				}
			}
			else if (args[curr].ToLower() == "-h" || args[curr].ToLower() == "--help" || args[curr].ToLower() == "/help")
			{
				DisplayHelp();
				return;
			}
			else
			{
				Console.WriteLine($"ERROR: Unrecognized parameter '{args[curr]}'!");
				DisplayHelp();
				return;
			}
		}

		// Set a default language binding
		if (lang == BindLang.None)
		{
			Console.WriteLine($"No language binding specified, defaulting to CSharp.");
			lang = BindLang.CSharp;
		}

		// Parse the files provided
		Console.WriteLine($"Parsing {files.Count} file(s)");
		CppCompilation ast = CppParser.ParseFiles(files);

		// And create bindings for it all!
		switch(lang)
		{
			case BindLang.CSharp: BindCSharp.Bind(ast, destFolder); break;
		}
	}

	///////////////////////////////////////////

	static void DisplayHelp()
	{
		Console.WriteLine($"Usage: {System.Reflection.Assembly.GetExecutingAssembly().GetName().Name}.exe [options]");
		Console.WriteLine(@"
    -o filepath   Override names. Specify a text file with a list of name
                  override pairs. Each line should have 1 source name, and 1
                  destination name. Comments begin with '#', and empty lines
                  are permitted.
    -f filepath   Header file. Specify the path to a C++ header file for
                  parsing and converting. You can provide more than one file
                  this way.
    -l language   Binding language. What bindings are generated from the the 
                  provided header files. Valid options are: CSharp.");
	}
}