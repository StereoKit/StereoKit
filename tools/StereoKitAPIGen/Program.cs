using System.Reflection;
using StereoKitAPIGen;

enum BindLang {
	None,
	CSharp,
	Zig,
	All,
}

class Program {
	static string FindStereoKitRoot() {
		string? exePath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
		string? path    = exePath;

		while (!string.IsNullOrEmpty(path)) {
			if (Path.GetFileName(path) == "StereoKitAPIGen")
				return path;
			path = Path.GetDirectoryName(path);
		}

		Console.WriteLine($"WARNING: Could not find StereoKitAPIGen root from exe path: {exePath}");
		return exePath ?? ".";
	}

	static void Main(string[] args) {
		string root = FindStereoKitRoot();

		// Default arguments if none provided
		if (args.Length == 0) {
			args = new string[] {
				"-f", Path.Combine(root, "..", "..", "StereoKitC", "stereokit.h"),
				"-f", Path.Combine(root, "..", "..", "StereoKitC", "stereokit_ui.h"),
				"-d", Path.Combine(root, "..", "..", "StereoKit", "Native"),
				"-l", "CSharp",
			};
		}

		// Parse command line arguments
		var files          = new List<string>();
		var lang           = BindLang.None;
		var destFolder     = "";
		var csOverrideFile = Path.Combine(root, "SKOverridesCSharp.txt");
		var zigOverrideFile= Path.Combine(root, "SKOverridesZig.txt");
		var moduleFile     = Path.Combine(root, "SKModules.txt");

		int curr = 0;
		while (curr < args.Length) {
			string arg = args[curr].ToLower();

			if (arg == "-f") {
				if (curr + 1 >= args.Length) { Error("Missing file path for -f"); return; }
				files.Add(args[++curr]);
				curr++;
			}
			else if (arg == "-d") {
				if (curr + 1 >= args.Length) { Error("Missing folder path for -d"); return; }
				destFolder = args[++curr];
				curr++;
			}
			else if (arg == "-l") {
				if (curr + 1 >= args.Length) { Error("Missing language for -l"); return; }
				if (!Enum.TryParse(args[++curr], true, out lang)) {
					Error($"Invalid language: {args[curr]}");
					return;
				}
				curr++;
			}
			else if (arg == "-ocs" || arg == "--override-csharp") {
				if (curr + 1 >= args.Length) { Error("Missing file path for C# overrides"); return; }
				csOverrideFile = args[++curr];
				curr++;
			}
			else if (arg == "-ozig" || arg == "--override-zig") {
				if (curr + 1 >= args.Length) { Error("Missing file path for Zig overrides"); return; }
				zigOverrideFile = args[++curr];
				curr++;
			}
			else if (arg == "-m" || arg == "--modules") {
				if (curr + 1 >= args.Length) { Error("Missing file path for modules"); return; }
				moduleFile = args[++curr];
				curr++;
			}
			else if (arg == "-h" || arg == "--help" || arg == "/help") {
				DisplayHelp();
				return;
			}
			else {
				Error($"Unrecognized argument: {args[curr]}");
				return;
			}
		}

		// Default to CSharp if no language specified
		if (lang == BindLang.None) {
			Console.WriteLine("No language specified, defaulting to CSharp.");
			lang = BindLang.CSharp;
		}

		// Verify files exist
		foreach (var f in files) {
			if (!File.Exists(f)) {
				Console.WriteLine($"WARNING: File not found: {f}");
			}
		}

		if (files.Count == 0) {
			Error("No input files specified!");
			return;
		}

		// Load module overrides
		var moduleOverrides = SKParser.LoadModuleOverrides(moduleFile);

		// Parse the header files
		Console.WriteLine($"Parsing {files.Count} file(s)...");
		var data = SKParser.Parse(files.ToArray(), moduleOverrides);

		Console.WriteLine($"Found: {data.Enums.Count} enums, {data.Modules.Count} modules, {data.Functions.Count} functions, {data.Constants.Count} constants");

		// Generate bindings
		if (lang == BindLang.CSharp || lang == BindLang.All) {
			var csOverrides = new NameOverrides();
			csOverrides.Load(csOverrideFile);
			BindCSharp.Bind(data, destFolder, csOverrides);
		}

		if (lang == BindLang.Zig || lang == BindLang.All) {
			var zigOverrides = new NameOverrides();
			zigOverrides.Load(zigOverrideFile);
			string zigFolder = lang == BindLang.All
				? Path.Combine(destFolder, "zig")
				: destFolder;
			BindZig.Bind(data, zigFolder, zigOverrides);
		}

		Console.WriteLine("Done!");
	}

	static void Error(string message) {
		Console.WriteLine($"ERROR: {message}");
		DisplayHelp();
	}

	static void DisplayHelp() {
		string name = Assembly.GetExecutingAssembly().GetName().Name ?? "StereoKitAPIGen";
		Console.WriteLine($@"
Usage: {name} [options]

Options:
  -f <path>       Input C header file (can specify multiple)
  -d <path>       Output directory for generated bindings
  -l <lang>       Target language: CSharp, Zig, or All (default: CSharp)
  -ocs <path>     C# name overrides file (default: SKOverridesCSharp.txt)
  -ozig <path>    Zig name overrides file (default: SKOverridesZig.txt)
  -m <path>       Module prefix overrides file (default: SKModules.txt)
  -h, --help      Show this help message

Example:
  {name} -f stereokit.h -l CSharp -d ./output
");
	}
}
