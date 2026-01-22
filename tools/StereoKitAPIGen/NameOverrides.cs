namespace StereoKitAPIGen;

/// <summary>
/// Manages name translation overrides for different target languages.
/// Each language can have its own override file with type/name mappings.
///
/// Supported directives:
/// - name NewName   : Rename a type/enum value
/// - @noimpl name   : Skip auto-generating this type/delegate (hand-written elsewhere)
/// </summary>
public class NameOverrides {
	readonly Dictionary<string, string> _overrides = new();
	readonly HashSet<string> _noImplTypes = new();

	/// <summary>
	/// Load overrides from a file. Format: source_name target_name (one per line).
	/// Lines starting with # are comments.
	/// Lines starting with @noimpl mark types to skip for code generation.
	/// </summary>
	public bool Load(string filePath) {
		_overrides.Clear();
		_noImplTypes.Clear();

		if (!File.Exists(filePath)) {
			Console.WriteLine($"WARNING: Override file not found: {filePath}");
			return false;
		}

		string[] lines;
		try {
			lines = File.ReadAllLines(filePath);
		} catch (Exception e) {
			Console.WriteLine($"ERROR: Name override file failed to load!\n> {filePath}\n{e}");
			return false;
		}

		for (int i = 0; i < lines.Length; i++) {
			string line = lines[i].Trim();
			if (string.IsNullOrEmpty(line) || line.StartsWith('#')) continue;

			string[] words = line.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries);

			// Handle @noimpl directive - skip generation for these types
			if (words[0] == "@noimpl") {
				foreach (var name in words.Skip(1)) {
					_noImplTypes.Add(name);
				}
				continue;
			}

	
			if (words.Length != 2) {
				Console.WriteLine($"WARNING: Override line has wrong format (need 2 words):\n#{i + 1}: {line}");
				continue;
			}

			_overrides[words[0]] = words[1];
		}

		Console.WriteLine($"Loaded {_overrides.Count} overrides from {Path.GetFileName(filePath)}");
		return true;
	}

	/// <summary>Check if code generation should be skipped for this type/delegate.</summary>
	public bool ShouldSkipImpl(string name) => _noImplTypes.Contains(name);

	/// <summary>Try to get an override for the given name.</summary>
	public bool TryGet(string name, out string outName)
		=> _overrides.TryGetValue(name, out outName!);

	/// <summary>Get override or return the original name.</summary>
	public string Get(string name)
		=> _overrides.TryGetValue(name, out var result) ? result : name;

	/// <summary>Check if an override exists for the given name.</summary>
	public bool Has(string name) => _overrides.ContainsKey(name);

	/// <summary>Number of overrides loaded.</summary>
	public int Count => _overrides.Count;
}

/// <summary>
/// Static helper for managing multiple language-specific override sets.
/// </summary>
public static class OverrideManager {
	static readonly Dictionary<string, NameOverrides> _languages = new();

	/// <summary>Load overrides for a specific language from a file.</summary>
	public static bool LoadForLanguage(string language, string filePath) {
		var overrides = new NameOverrides();
		if (overrides.Load(filePath)) {
			_languages[language] = overrides;
			return true;
		}
		return false;
	}

	/// <summary>Get the override set for a language.</summary>
	public static NameOverrides? GetOverrides(string language)
		=> _languages.TryGetValue(language, out var o) ? o : null;

	/// <summary>Try to get an override for a name in a specific language.</summary>
	public static bool TryGet(string language, string name, out string outName) {
		outName = name;
		return _languages.TryGetValue(language, out var o) && o.TryGet(name, out outName!);
	}

	/// <summary>Get override or return original name for a specific language.</summary>
	public static string Get(string language, string name) {
		if (_languages.TryGetValue(language, out var o))
			return o.Get(name);
		return name;
	}

	/// <summary>Clear all loaded overrides.</summary>
	public static void Clear() => _languages.Clear();
}
