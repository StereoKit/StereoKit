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
	public struct Matrix
	{
		internal Matrix4x4 m;

		public Matrix(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44)
			=> m = new Matrix4x4(
				m11, m12, m13, m14, 
				m21, m22, m23, m24, 
				m31, m32, m33, m34, 
				m41, m42, m43, m44);
		public Matrix(Matrix4x4 matrix)
			=> m = matrix;

		public static implicit operator Matrix(Matrix4x4 m) => new Matrix(m);
		public static implicit operator Matrix4x4(Matrix m) => m.m;

		public static Matrix operator *(Matrix a, Matrix b) => a.m*b.m;
		public static Vec3   operator *(Matrix a, Vec3   b) => Vector3.Transform(b.v, a.m);
		public static Ray    operator *(Matrix a, Ray    b) => a.Transform(b);

		/// <summary>An identity Matrix is the matrix equivalent of '1'! 
		/// Transforming anything by this will leave it at the exact same
		/// place.</summary>
		public static Matrix Identity => Matrix4x4.Identity;

		/// <summary>Creates an inverse matrix! If the matrix takes a point 
		/// from a -> b, then its inverse takes the point from b -> a.
		/// </summary>
		/// <returns>An inverse matrix of the current one.</returns>
		public Matrix Inverse { get { Matrix4x4.Invert(m, out Matrix4x4 result); return result; } }

		/// <summary>Inverts this Matrix! If the matrix takes a point from a
		/// -> b, then its inverse takes the point from b -> a.</summary>
		public void Invert() => Matrix4x4.Invert(m, out Matrix4x4 result);
		/// <summary>Transforms a point through the Matrix! This is basically 
		/// just multiplying a vector (x,y,z,1) with the Matrix.</summary>
		/// <param name="point">The point to transform.</param>
		/// <returns>The point transformed by the Matrix.</returns>
		public Vec3 Transform(Vec3 point) => Vector3.Transform(point.v, m);
		/// <summary> Transforms a point through the Matrix, but excluding 
		/// translation! This is great for transforming vectors that are 
		/// -directions- rather than points in space. Use this to transform 
		/// normals and directions. The same as multiplying (x,y,z,0) with 
		/// the Matrix.</summary>
		/// <param name="normal">The direction to transform.</param>
		/// <returns>The direction transformed by the Matrix.</returns>
		public Vec3 TransformNormal(Vec3 normal) 
			=> Vector3.TransformNormal(normal, m);
		/// <summary>Shorthand to transform a ray though the Matrix! This
		/// properly transforms the position with the point transform method,
		/// and the direction with the direction transform method. Does not
		/// normalize, nor does it preserve a normalized direction if the 
		/// Matrix contains scale data.</summary>
		/// <param name="ray">A ray you wish to transform from one space to
		/// another.</param>
		/// <returns>The transformed ray!</returns>
		public Ray Transform(Ray ray) 
			=> new Ray(Vector3.Transform(ray.position, m), Vector3.TransformNormal(ray.direction, m));

		/// <summary>Translate. Creates a translation Matrix!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <returns>A Matrix containing a simple translation!</returns>
		public static Matrix T  (Vector3 translation) 
			=> Matrix4x4.CreateTranslation(translation.X, translation.Y, translation.Z);
		/// <summary>Translate. Creates a translation Matrix!</summary>
		/// <param name="x">Move an object on the x axis by this amount.</param>
		/// <param name="y">Move an object on the y axis by this amount.</param>
		/// <param name="z">Move an object on the z axis by this amount.</param>
		/// <returns>A Matrix containing a simple translation!</returns>
		public static Matrix T (float x, float y, float z) 
			=> Matrix4x4.CreateTranslation(x,y,z);

		/// <summary>Create a rotation matrix from a Quaternion.</summary>
		/// <param name="rotation">A Quaternion describing the rotation for 
		/// this transform.</param>
		/// <returns>A Matrix that will rotate by the provided Quaternion 
		/// orientation.</returns>
		public static Matrix R (Quat rotation) 
			=> Matrix4x4.CreateFromQuaternion(rotation);
		/// <summary>Create a rotation matrix from pitch, yaw, and roll 
		/// information. Units are in degrees.</summary>
		/// <param name="pitchXDeg">Pitch, or rotation around the X axis, in
		/// degrees.</param>
		/// <param name="yawYDeg">Yaw, or rotation around the Y axis, in 
		/// degrees.</param>
		/// <param name="rollZDeg">Roll, or rotation around the Z axis, in
		/// degrees.</param>
		/// <returns>A Matrix that will rotate by the provided pitch, yaw and 
		/// roll.</returns>
		public static Matrix R (float pitchXDeg, float yawYDeg, float rollZDeg) 
			=> Matrix4x4.CreateFromYawPitchRoll(yawYDeg, pitchXDeg, rollZDeg);
		/// <summary>Create a rotation matrix from pitch, yaw, and roll 
		/// information. Units are in degrees.</summary>
		/// <param name="pitchYawRollDeg">Pitch (x-axis), yaw (y-axis), and 
		/// roll (z-axis) stored as x, y and z respectively in this Vec3.
		/// Units are in degrees.</param>
		/// <returns>A Matrix that will rotate by the provided pitch, yaw and 
		/// roll.</returns>
		public static Matrix R (Vec3 pitchYawRollDeg) 
			=> Matrix4x4.CreateFromYawPitchRoll(pitchYawRollDeg.y, pitchYawRollDeg.x, pitchYawRollDeg.z);

		/// <summary>Creates a scaling Matrix, where scale can be different
		/// on each axis (non-uniform).</summary>
		/// <param name="scale">How much larger or smaller this transform
		/// makes things. Vec3.One is a good default, as Vec3.Zero will
		/// shrink it to nothing!</param>
		/// <returns>A non-uniform scaling matrix.</returns>
		public static Matrix S (Vec3 scale) 
			=> Matrix4x4.CreateScale(scale.x, scale.y, scale.z);
		/// <summary>Creates a scaling Matrix, where the scale is the same on
		/// each axis (uniform).</summary>
		/// <param name="scale">How much larger or smaller this transform
		/// makes things. 1 is a good default, as 0 will shrink it to nothing!
		/// This will expand to a scale vector of (size, size, size)</param>
		/// <returns>A uniform scaling matrix.</returns>
		public static Matrix S (float scale) 
			=> Matrix4x4.CreateScale(scale, scale, scale);

		/// <summary>Translate, Scale. Creates a transform Matrix using both
		/// these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="scale">How much larger or smaller this transform
		/// makes things. 1 is a good default, as 0 will shrink it to nothing!
		/// This will expand to a scale vector of (size, size, size)</param>
		/// <returns>A Matrix that combines translation and scale information
		/// into a single Matrix!</returns>
		public static Matrix TS (Vector3 translation, float scale) 
			=> NativeAPI.matrix_trs(translation, Quat.Identity, new Vec3(scale, scale, scale));
		/// <summary>Translate, Scale. Creates a transform Matrix using both
		/// these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="scale">How much larger or smaller this transform 
		/// makes things. Vec3.One is a good default, as Vec3.Zero will 
		/// shrink it to nothing!</param>
		/// <returns>A Matrix that combines translation and scale information
		/// into a single Matrix!</returns>
		public static Matrix TS (Vec3 translation, Vec3 scale)
			=> NativeAPI.matrix_trs(translation, Quat.Identity, scale);
		/// <summary>Translate, Scale. Creates a transform Matrix using both
		/// these components!</summary>
		/// <param name="x">Move an object on the x axis by this amount.</param>
		/// <param name="y">Move an object on the y axis by this amount.</param>
		/// <param name="z">Move an object on the z axis by this amount.</param>
		/// <param name="scale">How much larger or smaller this transform 
		/// makes things. Vec3.One is a good default, as Vec3.Zero will 
		/// shrink it to nothing!</param>
		/// <returns>A Matrix that combines translation and scale information
		/// into a single Matrix!</returns>
		public static Matrix TS (float x, float y, float z, float scale) 
			=> NativeAPI.matrix_trs(new Vec3(x, y, z), Quat.Identity, new Vec3(scale, scale, scale));
		/// <summary>Translate, Scale. Creates a transform Matrix using both
		/// these components!</summary>
		/// <param name="x">Move an object on the x axis by this amount.</param>
		/// <param name="y">Move an object on the y axis by this amount.</param>
		/// <param name="z">Move an object on the z axis by this amount.</param>
		/// <param name="scale">How much larger or smaller this transform 
		/// makes things. Vec3.One is a good default, as Vec3.Zero will 
		/// shrink it to nothing!</param>
		/// <returns>A Matrix that combines translation and scale information
		/// into a single Matrix!</returns>
		public static Matrix TS (float x, float y, float z, Vec3  scale) 
			=> NativeAPI.matrix_trs(new Vec3(x, y, z), Quat.Identity, scale);


		/// <summary>Translate, Rotate. Creates a transform Matrix using 
		/// these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="rotation">A Quaternion describing the rotation for 
		/// this transform.</param>
		/// <returns>A Matrix that combines translation and rotation
		/// information into a single Matrix!</returns>
		public static Matrix TR (Vec3 translation, Quat rotation)
			=> NativeAPI.matrix_trs(translation, rotation, Vec3.One);
		/// <summary>Translate, Rotate. Creates a transform Matrix using 
		/// these components!</summary>
		/// <param name="x">Move an object on the x axis by this amount.</param>
		/// <param name="y">Move an object on the y axis by this amount.</param>
		/// <param name="z">Move an object on the z axis by this amount.</param>
		/// <param name="rotation">A Quaternion describing the rotation for 
		/// this transform.</param>
		/// <returns>A Matrix that combines translation and rotation
		/// information into a single Matrix!</returns>
		public static Matrix TR (float x, float y, float z, Quat rotation)
			=> NativeAPI.matrix_trs(new Vec3(x, y, z), rotation, Vec3.One);
		/// <summary>Translate, Rotate. Creates a transform Matrix using 
		/// these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="pitchYawRollDeg">Pitch (x-axis), yaw (y-axis), and 
		/// roll (z-axis) stored as x, y and z respectively in this Vec3.
		/// Units are in degrees.</param>
		/// <returns>A Matrix that combines translation and rotation
		/// information into a single Matrix!</returns>
		public static Matrix TR (Vec3 translation, Vec3 pitchYawRollDeg) 
			=> NativeAPI.matrix_trs(translation, Quat.FromAngles(pitchYawRollDeg), Vec3.One);
		/// <summary>Translate, Rotate. Creates a transform Matrix using 
		/// these components!</summary>
		/// <param name="x">Move an object on the x axis by this amount.</param>
		/// <param name="y">Move an object on the y axis by this amount.</param>
		/// <param name="z">Move an object on the z axis by this amount.</param>
		/// <param name="pitchYawRollDeg">Pitch (x-axis), yaw (y-axis), and 
		/// roll (z-axis) stored as x, y and z respectively in this Vec3.
		/// Units are in degrees.</param>
		/// <returns>A Matrix that combines translation and rotation
		/// information into a single Matrix!</returns>
		public static Matrix TR (float x, float y, float z, Vec3 pitchYawRollDeg) 
			=> NativeAPI.matrix_trs(new Vec3(x, y, z), Quat.FromAngles(pitchYawRollDeg), Vec3.One);
		
		/// <summary>Translate, Rotate, Scale. Creates a transform Matrix 
		/// using all these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="rotation">A Quaternion describing the rotation for 
		/// this transform.</param>
		/// <param name="scale">How much larger or smaller this transform 
		/// makes things. 1 is a good default, as 0 will shrink it to nothing! 
		/// This will expand to a scale vector of (size, size, size)</param>
		/// <returns>A Matrix that combines translation, rotation, and scale
		/// information into a single Matrix!</returns>
		public static Matrix TRS(Vector3 translation, Quaternion rotation, float scale) 
			=> NativeAPI.matrix_trs(translation, rotation, new Vec3(scale, scale, scale));
		/// <summary>Translate, Rotate, Scale. Creates a transform Matrix 
		/// using all these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="rotation">A Quaternion describing the rotation for 
		/// this transform.</param>
		/// <param name="scale">How much larger or smaller this transform 
		/// makes things. Vec3.One is a good default, as Vec3.Zero will 
		/// shrink it to nothing!</param>
		/// <returns>A Matrix that combines translation, rotation, and scale
		/// information into a single Matrix!</returns>
		public static Matrix TRS(Vec3 translation, Quat rotation, Vec3  scale) 
			=> NativeAPI.matrix_trs(translation, rotation, scale);
		/// <summary>Translate, Rotate, Scale. Creates a transform Matrix 
		/// using all these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="pitchYawRollDeg">Pitch (x-axis), yaw (y-axis), and 
		/// roll (z-axis) stored as x, y and z respectively in this Vec3.
		/// Units are in degrees.</param>
		/// <param name="scale">How much larger or smaller this transform 
		/// makes things. Vec3.One is a good default, as Vec3.Zero will 
		/// shrink it to nothing!</param>
		/// <returns>A Matrix that combines translation, rotation, and scale
		/// information into a single Matrix!</returns>
		public static Matrix TRS(Vec3 translation, Vec3 pitchYawRollDeg, float scale) 
			=> NativeAPI.matrix_trs(translation, Quat.FromAngles(pitchYawRollDeg), new Vec3(scale, scale, scale));
		/// <summary>Translate, Rotate, Scale. Creates a transform Matrix 
		/// using all these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="pitchYawRollDeg">Pitch (x-axis), yaw (y-axis), and 
		/// roll (z-axis) stored as x, y and z respectively in this Vec3.
		/// Units are in degrees.</param>
		/// <param name="scale">How much larger or smaller this transform 
		/// makes things. Vec3.One is a good default, as Vec3.Zero will 
		/// shrink it to nothing!</param>
		/// <returns>A Matrix that combines translation, rotation, and scale
		/// information into a single Matrix!</returns>
		public static Matrix TRS(Vec3 translation, Vec3 pitchYawRollDeg, Vec3 scale) 
			=> NativeAPI.matrix_trs(translation, Quat.FromAngles(pitchYawRollDeg), scale);
	}
}
