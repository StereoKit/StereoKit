using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>A light source used for creating SphericalHarmonics data.
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct SHLight
	{
		/// <summary>Direction to the light source. StereoKit expects this
		/// normalized, and anything it hands back is.</summary>
		public Vec3 directionTo;
		/// <summary>Color of the light in linear space! Values here can
		/// exceed 1.</summary>
		public Color color;
	}

	/// <summary>Spherical Harmonics are kinda like Fourier, but on a sphere.
	/// That doesn't mean terribly much to me, and could be wrong, but check
	/// out [here](http://www.ppsloan.org/publications/StupidSH36.pdf) for
	/// more details about how Spherical Harmonics work in this context!
	/// 
	/// However, the more practical thing is, SH can be a function that
	/// describes a value over the surface of a sphere! This is particularly
	/// useful for lighting, since you can basically store the lighting
	/// information for a space in this value! This is often used for
	/// lightmap data, or a light probe grid, but StereoKit just uses a
	/// single SH for the entire scene. It's a gross oversimplification, but
	/// looks quite good, and is really fast! That's extremely great when
	/// you're trying to hit 60fps, or even 144fps.
	///
	/// StereoKit's SH format at a glance, both for this CPU side struct, and
	/// for the GPU side representation:
	///
	/// | Choice        | SphericalHarmonics struct            | Shader constants          |
	/// |---------------|--------------------------------------|---------------------------|
	/// | Basis phase   | No Condon-Shortley phase             | Same                      |
	/// | Normalization | Orthonormal, constants at eval time  | Constants pre-baked       |
	/// | Domain        | Radiance, no cosine lobe             | Irradiance, lobe baked in |
	/// | Packing       | Bands ascending, RGB per coefficient | 7 Vec4s, dot-product form |
	///
	/// This struct is the CPU side format! Math and imports happen
	/// here, and StereoKit converts it to the baked shader form once per
	/// lighting change. With no phase factor, all basis signs are
	/// positive, so a light from above lands as a _positive_ `coefficient2`
	/// (the linear y term), and the linear band points _toward_ the
	/// brightest region. Watch out when importing coefficients from
	/// elsewhere as many libraries (ARCore, DirectXMath) include the phase
	/// factor, flipping the odd terms (coefficients 2, 4, 6, and 8)
	/// relative to this. If imported lighting shows up rotated 180
	/// degrees, with up reading as down, negate those four!</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct SphericalHarmonics
	{
		/// <summary>Band 0, the constant/ambient term.</summary>
		public Vec3 coefficient1;
		/// <summary>Band 1, the linear y term.</summary>
		public Vec3 coefficient2;
		/// <summary>Band 1, the linear z term.</summary>
		public Vec3 coefficient3;
		/// <summary>Band 1, the linear x term.</summary>
		public Vec3 coefficient4;
		/// <summary>Band 2, the xy term.</summary>
		public Vec3 coefficient5;
		/// <summary>Band 2, the yz term.</summary>
		public Vec3 coefficient6;
		/// <summary>Band 2, the 3z^2-1 term.</summary>
		public Vec3 coefficient7;
		/// <summary>Band 2, the xz term.</summary>
		public Vec3 coefficient8;
		/// <summary>Band 2, the x^2-y^2 term.</summary>
		public Vec3 coefficient9;

		/// <summary>The direction toward the strongest light source in this
		/// lighting data, following the same convention as
		/// `SHLight.directionTo`. The value is normalized, and sampling the
		/// light's color is `sh.Sample(sh.DominantLightDirectionTo)`.
		/// </summary>
		public Vec3 DominantLightDirectionTo => NativeAPI.sh_dominant_dir_to(this);

		/// <summary>The dominant directional light in this lighting data,
		/// the same derivation `Lighting.MainLight` uses! The direction is
		/// normalized, and the color is the projection matched directional
		/// component: the light you could remove from this SH, or add back
		/// on top of it.</summary>
		public SHLight DominantLight => NativeAPI.sh_dominant_light(this);

		/// <summary>The direction the dominant light is _traveling_,
		/// pointing away from the light source, the opposite of
		/// `SHLight.directionTo`.</summary>
		[Obsolete("Use -DominantLightDirectionTo instead. Note the negation!")]
		public Vec3 DominantLightDirection => -NativeAPI.sh_dominant_dir_to(this);

		/// <summary>Creates a SphericalHarmonic from an array of
		/// coefficients. Useful for loading stored data!</summary>
		/// <param name="coefficients">Must be an array with a length of 9!
		/// </param>
		public SphericalHarmonics(Vec3[] coefficients)
		{
			if (coefficients.Length != 9) throw new System.ArgumentException("coefficients must have 9 elements!");
			coefficient1 = coefficients[0];
			coefficient2 = coefficients[1];
			coefficient3 = coefficients[2];
			coefficient4 = coefficients[3];
			coefficient5 = coefficients[4];
			coefficient6 = coefficients[5];
			coefficient7 = coefficients[6];
			coefficient8 = coefficients[7];
			coefficient9 = coefficients[8];
		}

		/// <summary>Converts the SphericalHarmonic into an array of 
		/// coefficients 9 long. Useful for storing calculated data!</summary>
		/// <returns>An array of coefficients 9 long.</returns>
		public Vec3[] ToArray()
		{
			return new Vec3[] { 
				coefficient1,
				coefficient2,
				coefficient3,
				coefficient4,
				coefficient5,
				coefficient6,
				coefficient7,
				coefficient8,
				coefficient9 };
		}

		/// <summary>Look up the color information in a particular direction:
		/// the irradiance for a surface whose normal faces that way.</summary>
		/// <param name="normal">The direction to look in. Should be normalized.</param>
		/// <returns>The Color represented by the SH in the given direction.</returns>
		public Color Sample(Vec3 normal) => NativeAPI.sh_lookup(this, normal);

		/// <summary>Adds a 'directional light' to the lighting approximation.
		/// This can be used to bake a multiple light setup, or accumulate
		/// light from a field of points.</summary>
		/// <param name="lightDir">Direction to the light source.</param>
		/// <param name="lightColor">Color of the light, in linear color
		/// space. Values can exceed 1.</param>
		public void Add(Vec3 lightDir, Color lightColor)
			=> NativeAPI.sh_add(ref this, lightDir, new Vec3(lightColor.r, lightColor.g, lightColor.b));

		/// <summary>Removes a directional light from this lighting data,
		/// clamped to the light actually present so the result stays
		/// physical. Pairs with `DominantLight` for splitting the dominant
		/// light out of an SH, the way `Lighting.MainLight` does.</summary>
		/// <param name="light">The light to remove, such as one from
		/// `DominantLight`.</param>
		/// <returns>The light actually removed, which may be dimmer than
		/// requested when this SH doesn't contain that much light.</returns>
		public SHLight SubtractLight(SHLight light)
			=> NativeAPI.sh_subtract_light(ref this, light);

		/// <summary>Scales all the SphericalHarmonic's coefficients! This
		/// behaves as if you're modifying the brightness of the lighting
		/// this object represents.</summary>
		/// <param name="scale">A multiplier for the coefficients! A value of
		/// 1 will leave everything the same, 0.5 will cut the brightness in
		/// half, and a 2 will double the brightness.</param>
		public void Brightness(float scale)
			=> NativeAPI.sh_brightness(ref this, scale);

		/// <summary>Creates a SphericalHarmonics approximation of the
		/// irradiance given from a set of directional lights!</summary>
		/// <param name="directional_lights">A list of directional lights!</param>
		/// <returns>A SphericalHarmonics approximation of the irradiance
		/// given from a set of directional lights!</returns>
		public static SphericalHarmonics FromLights(SHLight[] directional_lights)
			=> NativeAPI.sh_create(directional_lights, directional_lights.Length);
	}
}
