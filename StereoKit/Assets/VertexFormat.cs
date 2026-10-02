// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>Describes how a field of a vertex struct maps to a component
	/// of a vertex format! Tag every field of your custom vertex struct with
	/// this attribute, and `Mesh.SetVerts&lt;T&gt;` will derive the complete
	/// vertex format from the struct via reflection.
	///
	/// The field's format is explicit rather than inferred from the field's
	/// type, so the field just needs to be the right size for the format.
	/// This means packed data like a pair of half floats can live in a plain
	/// `uint` field, with bit math or types like `System.Half` used to fill
	/// it in.
	///
	/// Vertex structs must be tightly packed, with no padding between fields
	/// or at the end of the struct. If your field sizes don't naturally align,
	/// use `[StructLayout(LayoutKind.Sequential, Pack = 1)]`.</summary>
	[AttributeUsage(AttributeTargets.Field, AllowMultiple = false)]
	public sealed class VertComponentAttribute : Attribute
	{
		/// <summary>What this component means, this is matched with the
		/// shader's vertex input semantics.</summary>
		public VertSemantic Semantic { get; }
		/// <summary>The data format of a single element of this component.
		/// </summary>
		public VertFmt Format { get; }
		/// <summary>How many format elements this component has, 1-4. A Vec3
		/// position would be 3.</summary>
		public int Count { get; }
		/// <summary>Distinguishes multiple components with the same semantic,
		/// like TEXCOORD0 vs TEXCOORD1. Usually 0.</summary>
		public int SemanticSlot { get; }

		/// <summary>Describe the vertex component this field contains.
		/// </summary>
		/// <param name="semantic">What this component means, this is matched
		/// with the shader's vertex input semantics.</param>
		/// <param name="format">The data format of a single element of this
		/// component.</param>
		/// <param name="count">How many format elements this component has,
		/// 1-4. A Vec3 position would be 3.</param>
		/// <param name="semanticSlot">Distinguishes multiple components with
		/// the same semantic, like TEXCOORD0 vs TEXCOORD1.</param>
		public VertComponentAttribute(VertSemantic semantic, VertFmt format, int count, int semanticSlot = 0)
		{
			Semantic     = semantic;
			Format       = format;
			Count        = count;
			SemanticSlot = semanticSlot;
		}
	}

	/// <summary>Derives and caches the vertex format of a vertex struct from
	/// its [VertComponent] attributes. Validation is aggressive, a layout
	/// that doesn't exactly describe the struct's memory throws with an
	/// explanation at first use.</summary>
	internal static class VertLayout<T> where T : unmanaged
	{
		static VertComponent[] _components;

		internal static VertComponent[] Components { get { if (_components == null) _components = Build(); return _components; } }

		static int FmtSize(VertFmt fmt) => fmt switch {
			VertFmt.F32 or VertFmt.I32 or VertFmt.U32 => 4,
			VertFmt.F16 or VertFmt.I16 or VertFmt.U16 or VertFmt.I16Normalized or VertFmt.U16Normalized => 2,
			VertFmt.I8 or VertFmt.U8 or VertFmt.I8Normalized or VertFmt.U8Normalized => 1,
			_ => 0,
		};

		static VertComponent[] Build()
		{
			Type        type   = typeof(T);
			FieldInfo[] fields = type.GetFields(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
			if (fields.Length == 0)
				throw new ArgumentException($"Vertex struct {type.Name} has no fields!");

			// Reflection makes no promises about field order, sort the
			// fields by their actual memory offsets.
			int[] offsets = new int[fields.Length];
			for (int i = 0; i < fields.Length; i++)
				offsets[i] = (int)Marshal.OffsetOf<T>(fields[i].Name);
			Array.Sort(offsets, fields);

			// Each field must start exactly where the previous components end,
			// one chain that catches padding and size mismatches alike.
			const string layoutHint = "Check that each field's size matches its [VertComponent], and that the struct is tightly packed, like with [StructLayout(LayoutKind.Sequential, Pack = 1)].";

			VertComponent[] result = new VertComponent[fields.Length];
			int             size   = 0;
			for (int i = 0; i < fields.Length; i++)
			{
				VertComponentAttribute attr = fields[i].GetCustomAttribute<VertComponentAttribute>() ??
					throw new ArgumentException($"Vertex struct field {type.Name}.{fields[i].Name} needs a [VertComponent] attribute!");
				if (attr.Semantic == VertSemantic.None || FmtSize(attr.Format) == 0 || attr.Count < 1 || attr.Count > 4)
					throw new ArgumentException($"Vertex struct field {type.Name}.{fields[i].Name} has an invalid [VertComponent] semantic, format, or count!");
				if (offsets[i] != size)
					throw new ArgumentException($"Vertex struct {type.Name}: field {fields[i].Name} starts at byte {offsets[i]}, but the components before it describe {size} bytes! {layoutHint}");

				result[i] = new VertComponent(attr.Semantic, attr.Format, attr.Count, attr.SemanticSlot);
				size     += FmtSize(attr.Format) * attr.Count;
			}

			int structSize = Marshal.SizeOf<T>();
			if (size != structSize)
				throw new ArgumentException($"Vertex struct {type.Name} is {structSize} bytes, but its components describe {size} bytes! {layoutHint}");

			return result;
		}
	}
}
