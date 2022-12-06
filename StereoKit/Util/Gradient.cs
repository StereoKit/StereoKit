using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>A color/position pair for Gradient values!</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct GradientKey
	{
		/// <summary>The color for this item, preferably in some form of 
		/// linear color space. Gamma corrected colors will definitely not
		/// math correctly.</summary>
		public Color color;
		/// <summary>Typically a value between 0-1! This is the position of 
		/// the color along the 'x-axis' of the gradient.</summary>
		public float position;

		/// <summary>A basic copy constructor for GradientKey.</summary>
		/// <param name="colorLinear">The color for this item, preferably in
		/// some form of linear color space. Gamma corrected colors will
		/// definitely not math correctly.</param>
		/// <param name="position">Typically a value between 0-1! This is the
		/// position of the color along the 'x-axis' of the gradient.</param>
		public GradientKey(Color colorLinear, float position)
		{
			this.position = position;
			this.color    = colorLinear;
		}
	}

	/// <summary>A Gradient is a sparse collection of color keys that are
	/// used to represent a ramp of colors! This class is largely just
	/// storing colors and allowing you to sample between them.
	/// 
	/// Since the Gradient is just interpolating values, you can use whatever 
	/// color space you want here, as long as it's linear and not gamma! 
	/// Gamma space RGB can't math properly at all. It can be RGB(linear),
	/// HSV, LAB, just remember which one you have, and be sure to convert it
	/// appropriately later. Data is stored as float colors, so this'll be a
	/// high accuracy blend!</summary>
	public class Gradient
	{
		internal IntPtr _inst;

		/// <summary>The number of color keys present in this gradient.</summary>
		public int Count => NativeAPI.gradient_count(_inst);

		/// <summary>Creates a new, completely empty gradient.</summary>
		public Gradient() => _inst = NativeAPI.gradient_create();
		/// <summary>Creates a new gradient from the list of color keys!</summary>
		/// <param name="keys">These can be in any order that you like, they'll
		/// be sorted by their GradientKey.position value regardless!</param>
		public Gradient(params GradientKey[] keys) => _inst = NativeAPI.gradient_create_keys(keys, keys.Length);
		/// <summary>Release reference to the StereoKit asset.</summary>
		~Gradient() => NativeAPI.gradient_release(_inst);

		/// <summary>This adds a color key into the list. It'll get inserted
		/// to the right slot based on its position.</summary>
		/// <param name="colorLinear">Any linear space color you like!</param>
		/// <param name="position">Typically a value between 0-1! This is the 
		/// position of the color along the 'x-axis' of the gradient.</param>
		public void    Add  (Color colorLinear, float position) => NativeAPI.gradient_add(_inst, colorLinear, position);
		/// <summary>Updates the color key at the given index! This will NOT 
		/// re-order color keys if they are moved past another key's position,
		/// which could lead to strange behavior.</summary>
		/// <param name="index">Index of the color key to change.</param>
		/// <param name="colorLinear">Any linear space color you like!</param>
		/// <param name="position">Typically a value between 0-1! This is the 
		/// position of the color along the 'x-axis' of the gradient.</param>
		public void    Set  (int index, Color colorLinear, float position) => NativeAPI.gradient_set(_inst, index, colorLinear, position);
		/// <summary>Removes the color key at the given index!</summary>
		/// <param name="index">Index of the color key to remove.</param>
		public void    Remove(int index) => NativeAPI.gradient_remove(_inst, index);
		/// <summary>Samples the gradient's color at the given position!</summary>
		/// <param name="at">Typically a value between 0-1, but if you used
		/// larger or smaller values for your color key's positions, it'll be
		/// in that range!</param>
		/// <returns>The interpolated color at the given position. If 'at' is
		/// smaller or larger than the gradient's position range, then the
		/// color will be clamped to the color at the beginning or end of the
		/// gradient!</returns>
		public Color   Get  (float at) => NativeAPI.gradient_get  (_inst, at);
		/// <summary>Samples the gradient's color at the given position, and
		/// converts it to a 32 bit color. If your RGBA color values are
		/// outside of the 0-1 range, then you'll get some issues as they're
		/// converted to 0-255 range bytes!</summary>
		/// <param name="at">Typically a value between 0-1, but if you used
		/// larger or smaller values for your color key's positions, it'll be
		/// in that range!</param>
		/// <returns>The interpolated 32 bit color at the given position. If
		/// 'at' is smaller or larger than the gradient's position range,
		/// then the color will be clamped to the color at the beginning or
		/// end of the gradient!</returns>
		public Color32 Get32(float at) => NativeAPI.gradient_get32(_inst, at);

	}
}
