using System;
using System.Collections.Generic;
using System.IO;

static class NameOverrides
{
	static Dictionary<string, string> overrides = new Dictionary<string, string>();

	///////////////////////////////////////////

	public static bool Load(string fileNamePairs)
	{
		overrides = new Dictionary<string, string>();

		string[] lines = null;
		try {
			lines = File.ReadAllLines(fileNamePairs);
		} catch(Exception e) {
			Console.WriteLine($"ERROR: Name override file failed to load!\n> {fileNamePairs}");
			return false;
		}

		for (int i = 0; i < lines.Length; i++)
		{
			string line = lines[i].Trim();
			if (string.IsNullOrEmpty(line) || line.StartsWith("#")) continue;

			string[] words = line.Split(' ');
			if (words.Length != 2)
			{
				Console.WriteLine($"WARNING: Name override file line has the wrong number of words on it, should only be 2!\n#{i}: {line}");
				continue;
			}

			overrides[words[0]] = words[1];
		}
		return true;
	}

	///////////////////////////////////////////

	public static bool TryGet(string name, out string outName)
		=> overrides.TryGetValue(name, out outName);
}