// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

namespace StereoKit
{
	/// <summary>This class contains some tools for hashing data within
	/// StereoKit! Certain systems in StereoKit use string hashes instead of
	/// full strings for faster search and compare, like UI and Materials, so
	/// this class gives access to the code SK uses for hashing.
	/// 
	/// StereoKit currently internally uses a 64 bit FNV hash, though this
	/// detail should be pretty transparent to developers.</summary>
	public static class Hash
	{
		/// <summary>This will hash the UTF8 representation of the given string
		/// into a hash value that StereoKit can use.</summary>
		/// <param name="str">A C# string that will be converted to UTF8, and
		/// then hashed.</param>
		/// <returns>A StereoKit hash representing the provided string.</returns>
		public static IdHash String(string str)
			=> NativeAPI.hash_string (str);

		/// <summary>This will hash the UTF8 representation of the given string
		/// into a hash value that StereoKit can use. This overload allows you
		/// to combine your hash with an existing hash.</summary>
		/// <param name="str">A C# string that will be converted to UTF8, and
		/// then hashed.</param>
		/// <param name="root">The hash value this new hash will start from.
		/// </param>
		/// <returns>A StereoKit hash representing a combination of the
		/// provided string and the root hash.</returns>
		public static IdHash String(string str, IdHash root)
			=> NativeAPI.hash_string_with(str, root);

		/// <summary>This will hash an integer into a hash value that StereoKit
		/// can use. This is helpful for adding in some uniqueness using
		/// something like a for loop index. This may be best when combined
		/// with additional hashes.</summary>
		/// <param name="val">An integer that will be hashed.</param>
		/// <returns>A StereoKit hash representing the provided integer.</returns>
		public static IdHash Int(int val)
				=> NativeAPI.hash_int(val);

		/// <summary>This will hash an integer into a hash value that StereoKit
		/// can use. This is helpful for adding in some uniqueness using
		/// something like a for loop index. This overload allows you to
		/// combine your hash with an existing hash.</summary>
		/// <param name="val">An integer that will be hashed.</param>
		/// <param name="root">The hash value this new hash will start from.
		/// </param>
		/// <returns>A StereoKit hash representing a combination of the
		/// provided string and the root hash.</returns>
		public static IdHash Int(int val, IdHash root)
			=> NativeAPI.hash_int_with(val, root);
	}
}
