using System.Numerics;

namespace StereoKit
{
	/// <summary>A Matrix in StereoKit is a 4x4 grid of numbers that is used 
	/// to represent a transformation for any sort of position or vector! 
	/// This is an oversimplification of what a matrix actually is, but it's
	/// accurate in this case.
	/// 
	/// Matrices are really useful for transforms because you can chain 
	/// together all sorts of transforms into a single Matrix! A Matrix
	/// transform really shines when applied to many positions, as the more
	/// expensive operations get cached within the matrix values.
	/// 
	/// Matrices are prominently used within shaders for mesh transforms!
	/// </summary>
	public static class Matrix
	{
		/// <summary>An identity Matrix is the matrix equivalent of '1'! 
		/// Transforming anything by this will leave it at the exact same
		/// place.</summary>
		public static Matrix4x4 Identity => Matrix4x4.Identity;

		/// <summary>Translate. Creates a translation Matrix!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <returns>A Matrix containing a simple translation!</returns>
		public static Matrix4x4 T  (Vector3 translation)       => Matrix4x4.CreateTranslation(translation.X, translation.Y, translation.Z);
		/// <summary>Translate. Creates a translation Matrix!</summary>
		/// <param name="x">Move an object on the x axis by this amount.</param>
		/// <param name="y">Move an object on the y axis by this amount.</param>
		/// <param name="z">Move an object on the z axis by this amount.</param>
		/// <returns>A Matrix containing a simple translation!</returns>
		public static Matrix4x4 T  (float x, float y, float z) => Matrix4x4.CreateTranslation(x,y,z);

		public static Matrix4x4 R  (Quaternion rotation)                            => Matrix4x4.CreateFromQuaternion(rotation);
		public static Matrix4x4 R  (float pitchXDeg, float yawYDeg, float rollZDeg) => Matrix4x4.CreateFromYawPitchRoll(yawYDeg, pitchXDeg, rollZDeg);
		public static Matrix4x4 R  (Vector3 pitchYawRollDeg)                        => Matrix4x4.CreateFromYawPitchRoll(pitchYawRollDeg.Y, pitchYawRollDeg.X, pitchYawRollDeg.Z);
		public static Matrix4x4 S  (Vector3 scale) => Matrix4x4.CreateScale(scale.X, scale.Y, scale.Z);
		public static Matrix4x4 S  (float   scale) => Matrix4x4.CreateScale(scale, scale, scale);
		/// <summary>Translate, Scale. Creates a transform Matrix using both
		/// these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="scale">How much larger or smaller this transform
		/// makes things. 1 is a good default, as 0 will shrink it to nothing!
		/// This will expand to a scale vector of (size, size, size)</param>
		/// <returns>A Matrix that combines translation and scale information
		/// into a single Matrix!</returns>
		public static Matrix4x4 TS (Vector3 translation,       float   scale) => NativeAPI.matrix_trs(translation, Quaternion.Identity, new Vector3(scale, scale, scale));
		/// <summary>Translate, Scale. Creates a transform Matrix using both
		/// these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="scale">How much larger or smaller this transform 
		/// makes things. Vec3.One is a good default, as Vec3.Zero will 
		/// shrink it to nothing!</param>
		/// <returns>A Matrix that combines translation and scale information
		/// into a single Matrix!</returns>
		public static Matrix4x4 TS (Vector3 translation,       Vector3 scale) => NativeAPI.matrix_trs(translation, Quaternion.Identity, scale);
		public static Matrix4x4 TS (float x, float y, float z, float   scale) => NativeAPI.matrix_trs(new Vector3(x, y, z), Quaternion.Identity, new Vector3(scale, scale, scale));
		public static Matrix4x4 TS (float x, float y, float z, Vector3 scale) => NativeAPI.matrix_trs(new Vector3(x, y, z), Quaternion.Identity, scale);
		public static Matrix4x4 TR (Vector3 translation,       Quaternion rotation)     => NativeAPI.matrix_trs(translation,          rotation, Vec3.One);
		public static Matrix4x4 TR (float x, float y, float z, Quaternion rotation)     => NativeAPI.matrix_trs(new Vector3(x, y, z), rotation, Vec3.One);
		public static Matrix4x4 TR (Vector3 translation,       Vector3 pitchYawRollDeg) => NativeAPI.matrix_trs(translation,          Quat.FromAngles(pitchYawRollDeg), Vec3.One);
		public static Matrix4x4 TR (float x, float y, float z, Vector3 pitchYawRollDeg) => NativeAPI.matrix_trs(new Vector3(x, y, z), Quat.FromAngles(pitchYawRollDeg), Vec3.One);
		
		/// <summary>Translate, Rotate, Scale. Creates a transform Matrix 
		/// using all these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="rotation">A Quaternion describing the rotation for 
		/// this transform.</param>
		/// <param name="scale">How much larger or smaller this transform 
		/// makes things. 1 is a good default, as 0 will shrink it to nothing! 
		/// This will expand to a scale vector of (size, size, size)</param>
		/// <returns>A Matrix that combines translation, roatation, and scale
		/// information into a single Matrix!</returns>
		public static Matrix4x4 TRS(Vector3 translation, Quaternion rotation,     float   scale) => NativeAPI.matrix_trs(translation, rotation, new Vector3(scale, scale, scale));
		/// <summary>Translate, Rotate, Scale. Creates a transform Matrix 
		/// using all these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="rotation">A Quaternion describing the rotation for 
		/// this transform.</param>
		/// <param name="scale">How much larger or smaller this transform 
		/// makes things. Vec3.One is a good default, as Vec3.Zero will 
		/// shrink it to nothing!</param>
		/// <returns>A Matrix that combines translation, roatation, and scale
		/// information into a single Matrix!</returns>
		public static Matrix4x4 TRS(Vector3 translation, Quaternion rotation,     Vector3 scale) => NativeAPI.matrix_trs(translation, rotation, scale);
		public static Matrix4x4 TRS(Vector3 translation, Vector3 pitchYawRollDeg, float   scale) => NativeAPI.matrix_trs(translation, Quat.FromAngles(pitchYawRollDeg), new Vector3(scale, scale, scale));
		public static Matrix4x4 TRS(Vector3 translation, Vector3 pitchYawRollDeg, Vector3 scale) => NativeAPI.matrix_trs(translation, Quat.FromAngles(pitchYawRollDeg), scale);

		/// <summary>Creates an inverse matrix! If the matrix takes a point 
		/// from a -> b, then its inverse takes the point from b -> a.</summary>
		/// <returns>An inverse matrix of the current one.</returns>
		public static Matrix4x4 Inverse        (this Matrix4x4 m) { Matrix4x4.Invert(m, out Matrix4x4 result); return result; }
		/// <summary>Transforms a point through the Matrix! This is basically 
		/// just multiplying a vector (x,y,z,1) with the Matrix.</summary>
		/// <param name="point">The point to transform.</param>
		/// <returns>The point transformed by the Matrix.</returns>
		public static Vector3 Transform(this Matrix4x4 m, Vector3 point ) => Vector3.Transform(point, m);
		/// <summary> Transforms a point through the Matrix, but excluding 
		/// translation! This is great for transforming vectors that are 
		/// -directions- rather than points in space. Use this to transform 
		/// normals and directions. The same as multiplying (x,y,z,0) with 
		/// the Matrix.</summary>
		/// <param name="normal">The direction to transform.</param>
		/// <returns>The direction transformed by the Matrix.</returns>
		public static Vector3 TransformNormal(this Matrix4x4 m, Vector3 normal) => Vector3.TransformNormal(normal, m);
		/// <summary>Shorthand to transform a ray though the Matrix! This
		/// properly transforms the position with the point transform method,
		/// and the direction with the direction transform method. Does not
		/// normalize, nor does it preserve a normalized direction if the 
		/// Matrix contains scale data.</summary>
		/// <param name="ray">A ray you wish to transform from one space to
		/// another.</param>
		/// <returns>The transformed ray!</returns>
		public static Ray Transform(this Matrix4x4 m, Ray ray) => new Ray(Vector3.Transform(ray.position, m), Vector3.TransformNormal(ray.direction, m));
	}
}
