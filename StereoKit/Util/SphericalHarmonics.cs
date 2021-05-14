using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>A light source used for creating SphericalHarmonics data.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct SHLight
	{
		/// <summary>Direction to the light source.</summary>
		public Vec3 directionTo;
		/// <summary>Color of the light in linear space! Values here can exceed 1.</summary>
		public Color color;
	}

	/// <summary>Spherical Harmonics are kinda like Fourier, but on a sphere. That
	/// doesn't mean terribly much to me, and could be wrong, but check out [here](http://www.ppsloan.org/publications/StupidSH36.pdf)
	/// for more details about how Spherical Harmonics work in this context!
	/// 
	/// However, the more prctical thing is, SH can be a function that describes a
	/// value over the surface of a sphere! This is particularly useful for lighting,
	/// since you can basically store the lighting information for a space in this 
	/// value! This is often used for lightmap data, or a light probe grid, but StereoKit
	/// just uses a single SH for the entire scene. It's a gross oversimplification, 
	/// but looks quite good, and is really fast! That's extremely great when you're 
	/// trying to hit 60fps, or even 144fps.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct SphericalHarmonics
	{
		public Vec3 coefficient1;
		public Vec3 coefficient2;
		public Vec3 coefficient3;
		public Vec3 coefficient4;
		public Vec3 coefficient5;
		public Vec3 coefficient6;
		public Vec3 coefficient7;
		public Vec3 coefficient8;
		public Vec3 coefficient9;


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

		/// <summary>Look up the color information in a particular direction!</summary>
		/// <param name="normal">The direction to look in. Should be normalized.</param>
		/// <returns>The Color represented by the SH in the given direction.</returns>
		public Color Sample(Vec3 normal) => NativeAPI.sh_lookup(this, normal);

		public void Add(Vec3 lightDir, Color lightColor)
			=> NativeAPI.sh_add(ref this, lightDir, new Vec3(lightColor.r, lightColor.g, lightColor.b));

		public void Brightness(float scale)
			=> NativeAPI.sh_brightness(ref this, scale);

		/// <summary>Creates a SphericalHarmonics approximation of the irradiance given
		/// from a set of directional lights!</summary>
		/// <param name="directional_lights">A list of directional lights!</param>
		/// <returns>A SphericalHarmonics approximation of the irradiance given
		/// from a set of directional lights!</returns>
		public static SphericalHarmonics FromLights(SHLight[] directional_lights)
			=> NativeAPI.sh_create(directional_lights, directional_lights.Length);
	}
}
