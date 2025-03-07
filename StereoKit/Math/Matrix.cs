﻿// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

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
	/// Multiple matrix transforms can be combined by multiplying them. In
	/// StereoKit, to create a matrix that first scales an object, followed by
	/// rotating it, and finally translating it you would use this order:
	/// 
	/// `Matrix M = Matrix.S(...) * Matrix.R(...) * Matrix.T(...);`
	/// 
	/// This order is related to the fact that StereoKit uses row-major order
	/// to store matrices. Note that in other 3D frameworks and certain 3D math
	/// references you may find column-major matrices, which would need the
	/// reverse order (i.e. T*R*S), so please keep this in mind when creating
	/// transformations.
	///
	/// Matrices are prominently used within shaders for mesh transforms!
	/// </summary>
	public struct Matrix
	{
		/// <summary>The internal, wrapped System.Numerics type. This can be
		/// nice to have around so you can pass its fields as 'ref', which you
		/// can't do with properties. You won't often need this, as implicit
		/// conversions to System.Numerics types are also provided.</summary>
		public Matrix4x4 m;

		/// <summary>This constructor is for manually creating a matrix from a
		/// grid of floats! You'll likely want to use one of the static Matrix
		/// functions to create a Matrix instead.</summary>
		/// <param name="m11">m11</param>
		/// <param name="m12">m12</param>
		/// <param name="m13">m13</param>
		/// <param name="m14">m14</param>
		/// <param name="m21">m21</param>
		/// <param name="m22">m22</param>
		/// <param name="m23">m23</param>
		/// <param name="m24">m24</param>
		/// <param name="m31">m31</param>
		/// <param name="m32">m32</param>
		/// <param name="m33">m33</param>
		/// <param name="m34">m34</param>
		/// <param name="m41">m41</param>
		/// <param name="m42">m42</param>
		/// <param name="m43">m43</param>
		/// <param name="m44">m44</param>
		public Matrix(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44)
			=> m = new Matrix4x4(
				m11, m12, m13, m14,
				m21, m22, m23, m24,
				m31, m32, m33, m34,
				m41, m42, m43, m44);
		/// <summary>Create a Matrix from the equivalent System.Numerics Matrix
		/// type. You can also implicitly convert between these types, as this
		/// Matrix is backed by the System.Numerics type anyhow.</summary>
		/// <param name="matrix">A System.Numerics matrix.</param>
		public Matrix(Matrix4x4 matrix)
			=> m = matrix;

		/// <summary>Allows implicit conversion from System.Numerics.Matrix4x4
		/// to StereoKit.Matrix.</summary>
		/// <param name="m">Any System.Numerics Matrix4x4.</param>
		/// <returns>A StereoKit compatible matrix.</returns>
		public static implicit operator Matrix(Matrix4x4 m) => new Matrix(m);
		/// <summary>Allows implicit conversion from StereoKit.Matrix to
		/// System.Numerics.Matrix4x4</summary>
		/// <param name="m">Any StereoKit.Matrix.</param>
		/// <returns>A System.Numerics compatible matrix.</returns>
		public static implicit operator Matrix4x4(Matrix m) => m.m;

		/// <summary>Multiplies two matrices together! This is a great way to
		/// combine transform operations. Note that StereoKit's matrices are
		/// row-major, and multiplication order is important! To translate,
		/// then scale, multiply in order of 'translate * scale'.</summary>
		/// <param name="a">First Matrix.</param>
		/// <param name="b">Second Matrix.</param>
		/// <returns>Result of matrix multiplication.</returns>
		public static Matrix operator *(Matrix a, Matrix b) => a.m*b.m;
		/// <summary>Multiplies the vector by the Matrix! Since only a 1x4
		/// vector can be multiplied against a 4x4 matrix, this uses '1' for
		/// the 4th element, so the result will also include translation! To
		/// exclude translation, use `Matrix.TransformNormal`.
		/// </summary>
		/// <param name="a">A transform matrix.</param>
		/// <param name="b">Any Vector.</param>
		/// <returns>The Vec3 transformed by the matrix, including translation.
		/// </returns>
		public static Vec3 operator *(Matrix a, Vec3 b) => Vector3.Transform(b.v, a.m);
		/// <summary>Transforms a Ray by the Matrix! The position and direction
		/// are both multiplied by the matrix, accounting properly for which
		/// should include translation, and which should not.</summary>
		/// <param name="a">A transform matrix.</param>
		/// <param name="b">A Ray to be transformed.</param>
		/// <returns>A Ray transformed by the Matrix.</returns>
		public static Ray operator *(Matrix a, Ray b) => a.Transform(b);
		/// <summary>Transforms a Pose by the Matrix! The position and
		/// orientation are both transformed by the matrix, accounting properly
		/// for the Pose's quaternion.</summary>
		/// <param name="a">A transform matrix.</param>
		/// <param name="b">A Pose to be transformed.</param>
		/// <returns>A Ray transformed by the Matrix.</returns>
		public static Pose operator *(Matrix a, Pose   b) => a.Transform(b);

		/// <summary>An identity Matrix is the matrix equivalent of '1'! 
		/// Transforming anything by this will leave it at the exact same
		/// place.</summary>
		public static Matrix Identity => Matrix4x4.Identity;

		/// <summary>A fast Property that will return or set the translation
		/// component embedded in this transform matrix.</summary>
		public Vec3 Translation { get => m.Translation; set => m.Translation = value; }
		/// <summary>Returns the scale embedded in this transform matrix. Not
		/// exactly cheap, requires 3 sqrt calls, but is cheaper than calling
		/// Decompose.</summary>
		public Vec3 Scale => NativeAPI.matrix_extract_scale(m);
		/// <summary>A slow function that returns the rotation quaternion 
		/// embedded in this transform matrix. This is backed by Decompose,
		/// so if you need any additional info, it's better to just call
		/// Decompose instead.</summary>
		public Quat Rotation => NativeAPI.matrix_extract_rotation(m);
		/// <summary>Extracts translation and rotation information from the
		/// transform matrix, and makes a Pose from it! Not exactly fast.
		/// This is backed by Decompose, so if you need any additional info,
		/// it's better to just call Decompose instead.</summary>
		public Pose Pose => NativeAPI.matrix_extract_pose(m);

		/// <summary>Creates an inverse matrix! If the matrix takes a point 
		/// from a -> b, then its inverse takes the point from b -> a.
		/// </summary>
		/// <returns>An inverse matrix of the current one.</returns>
		public Matrix Inverse { get { Matrix4x4.Invert(m, out Matrix4x4 result); return result; } }

		/// <summary>Creates a matrix that has been transposed! Transposing is
		/// like rotating the matrix 90 clockwise, or turning the rows into
		/// columns. This can be useful for inverting orthogonal matrices, or
		/// converting matrices for use in a math library that uses different
		/// conventions!</summary>
		public Matrix Transposed => Matrix4x4.Transpose(m);

		/// <summary>Inverts this Matrix! If the matrix takes a point from a
		/// -> b, then its inverse takes the point from b -> a.</summary>
		public void Invert() => Matrix4x4.Invert(m, out m);

		/// <summary>Transposes this Matrix! Transposing is like rotating the
		/// matrix 90 clockwise, or turning the rows into columns. This can be
		/// useful for inverting orthogonal matrices, or converting matrices
		/// for use in a math library that uses different conventions!
		/// </summary>
		public void Transpose() => this = Matrix4x4.Transpose(m);

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
		/// <summary>Shorthand for transforming a Pose! This will transform
		/// the position of the Pose with the matrix, extract a rotation Quat
		/// from the matrix and apply that to the Pose's orientation. Note
		/// that extracting a rotation Quat is an expensive operation, so if
		/// you're doing it more than once, you should cache the rotation 
		/// Quat and do this transform manually.</summary>
		/// <param name="pose">The original pose.</param>
		/// <returns>The transformed pose.</returns>
		public Pose Transform(Pose pose)
			=> NativeAPI.matrix_transform_pose(m, pose);

		/// <summary>Returns this transformation matrix to its original 
		/// translation, rotation and scale components. Not exactly a cheap
		/// function. If this is not a transform matrix, there's a chance
		/// this call will fail, and return false.</summary>
		/// <param name="translation">XYZ translation of the matrix.</param>
		/// <param name="rotation">The rotation quaternion, some lossiness
		/// may be encountered when composing/decomposing.</param>
		/// <param name="scale">XYZ scale components.</param>
		/// <returns>If this is not a transform matrix, there's a chance this
		/// call will fail, and return false.</returns>
		public bool Decompose(out Vec3 translation, out Quat rotation, out Vec3 scale)
			=> Matrix4x4.Decompose(m, out scale.v, out rotation.q, out translation.v);

		/// <summary>Translate. Creates a translation Matrix!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <returns>A Matrix containing a simple translation!</returns>
		public static Matrix T(Vec3 translation) 
			=> Matrix4x4.CreateTranslation(translation.v.X, translation.v.Y, translation.v.Z);
		/// <summary>Translate. Creates a translation Matrix!</summary>
		/// <param name="x">Move an object on the x axis by this amount.</param>
		/// <param name="y">Move an object on the y axis by this amount.</param>
		/// <param name="z">Move an object on the z axis by this amount.</param>
		/// <returns>A Matrix containing a simple translation!</returns>
		public static Matrix T(float x, float y, float z) 
			=> Matrix4x4.CreateTranslation(x,y,z);

		/// <summary>Create a rotation matrix from a Quaternion.</summary>
		/// <param name="rotation">A Quaternion describing the rotation for 
		/// this transform.</param>
		/// <returns>A Matrix that will rotate by the provided Quaternion 
		/// orientation.</returns>
		public static Matrix R(Quat rotation)
			=> Matrix4x4.CreateFromQuaternion(rotation.q);
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
		public static Matrix R(float pitchXDeg, float yawYDeg, float rollZDeg) 
			=> Matrix4x4.CreateFromYawPitchRoll(yawYDeg*Units.deg2rad, pitchXDeg*Units.deg2rad, rollZDeg*Units.deg2rad);
		/// <summary>Create a rotation matrix from pitch, yaw, and roll 
		/// information. Units are in degrees.</summary>
		/// <param name="pitchYawRollDeg">Pitch (x-axis), yaw (y-axis), and 
		/// roll (z-axis) stored as x, y and z respectively in this Vec3.
		/// Units are in degrees.</param>
		/// <returns>A Matrix that will rotate by the provided pitch, yaw and 
		/// roll.</returns>
		public static Matrix R(Vec3 pitchYawRollDeg) 
			=> Matrix4x4.CreateFromYawPitchRoll(pitchYawRollDeg.y*Units.deg2rad, pitchYawRollDeg.x*Units.deg2rad, pitchYawRollDeg.z*Units.deg2rad);

		/// <summary>Creates a scaling Matrix, where scale can be different
		/// on each axis (non-uniform).</summary>
		/// <param name="scale">How much larger or smaller this transform
		/// makes things. Vec3.One is a good default, as Vec3.Zero will
		/// shrink it to nothing!</param>
		/// <returns>A non-uniform scaling matrix.</returns>
		public static Matrix S(Vec3 scale)
			=> Matrix4x4.CreateScale(scale.x, scale.y, scale.z);
		/// <summary>Creates a scaling Matrix, where scale can be different
		/// on each axis (non-uniform).</summary>
		/// <param name="x">How much larger or smaller this transform makes
		/// things. 1 is a good default, as 0 will shrink it to nothing!</param>
		/// <returns>A non-uniform scaling matrix.</returns>
		public static Matrix S(float x, float y, float z)
			=> Matrix4x4.CreateScale(x, y, z);
		/// <summary>Creates a scaling Matrix, where the scale is the same on
		/// each axis (uniform).</summary>
		/// <param name="scale">How much larger or smaller this transform
		/// makes things. 1 is a good default, as 0 will shrink it to nothing!
		/// This will expand to a scale vector of (size, size, size)</param>
		/// <returns>A uniform scaling matrix.</returns>
		public static Matrix S(float scale)
			=> Matrix4x4.CreateScale(scale, scale, scale);

		/// <summary>Translate, Scale. Creates a transform Matrix using both
		/// these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="scale">How much larger or smaller this transform
		/// makes things. 1 is a good default, as 0 will shrink it to nothing!
		/// This will expand to a scale vector of (size, size, size)</param>
		/// <returns>A Matrix that combines translation and scale information
		/// into a single Matrix!</returns>
		public static Matrix TS(Vec3 translation, float scale)
			=> new Matrix4x4(
				scale, 0, 0, 0,
				0, scale, 0, 0,
				0, 0, scale, 0,
				translation.v.X, translation.v.Y, translation.v.Z, 1);
		/// <summary>Translate, Scale. Creates a transform Matrix using both
		/// these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="scale">How much larger or smaller this transform 
		/// makes things. Vec3.One is a good default, as Vec3.Zero will 
		/// shrink it to nothing!</param>
		/// <returns>A Matrix that combines translation and scale information
		/// into a single Matrix!</returns>
		public static Matrix TS(Vec3 translation, Vec3 scale)
			=> new Matrix4x4(
				scale.v.X, 0, 0, 0,
				0, scale.v.Y, 0, 0,
				0, 0, scale.v.Z, 0,
				translation.v.X, translation.v.Y, translation.v.Z, 1);
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
		public static Matrix TS(float x, float y, float z, float scale)
			=> new Matrix4x4(
				scale, 0, 0, 0,
				0, scale, 0, 0,
				0, 0, scale, 0,
				x, y, z, 1);
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
		public static Matrix TS(float x, float y, float z, Vec3  scale)
			=> new Matrix4x4(
				scale.v.X, 0, 0, 0,
				0, scale.v.Y, 0, 0,
				0, 0, scale.v.Z, 0,
				x, y, z, 1);


		/// <summary>Translate, Rotate. Creates a transform Matrix using 
		/// these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="rotation">A Quaternion describing the rotation for 
		/// this transform.</param>
		/// <returns>A Matrix that combines translation and rotation
		/// information into a single Matrix!</returns>
		public static Matrix TR(Vec3 translation, Quat rotation)
			=> TR(translation.v.X, translation.v.Y, translation.v.Z, rotation);
		/// <summary>Translate, Rotate. Creates a transform Matrix using 
		/// these components!</summary>
		/// <param name="x">Move an object on the x axis by this amount.</param>
		/// <param name="y">Move an object on the y axis by this amount.</param>
		/// <param name="z">Move an object on the z axis by this amount.</param>
		/// <param name="rotation">A Quaternion describing the rotation for 
		/// this transform.</param>
		/// <returns>A Matrix that combines translation and rotation
		/// information into a single Matrix!</returns>
		public static Matrix TR(float x, float y, float z, Quat rotation)
		{
			Matrix4x4 result = Matrix4x4.CreateFromQuaternion(rotation.q);
			result.M41 = x;
			result.M42 = y;
			result.M43 = z;
			return result;
		}
		/// <summary>Translate, Rotate. Creates a transform Matrix using 
		/// these components!</summary>
		/// <param name="translation">Move an object by this amount.</param>
		/// <param name="pitchYawRollDeg">Pitch (x-axis), yaw (y-axis), and 
		/// roll (z-axis) stored as x, y and z respectively in this Vec3.
		/// Units are in degrees.</param>
		/// <returns>A Matrix that combines translation and rotation
		/// information into a single Matrix!</returns>
		public static Matrix TR(Vec3 translation, Vec3 pitchYawRollDeg)
			=> TR(translation.v.X, translation.v.Y, translation.v.Z, Quat.FromAngles(pitchYawRollDeg));
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
		public static Matrix TR(float x, float y, float z, Vec3 pitchYawRollDeg)
			=> TR(x, y, z, Quat.FromAngles(pitchYawRollDeg));

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
		public static Matrix TRS(Vec3 translation, Quat rotation, float scale)
			=> TRS(translation, rotation, new Vec3(scale, scale, scale));
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
		public static Matrix TRS(Vec3 translation, Quat rotation, Vec3 scale)
		{
			Matrix4x4 result =
				Matrix4x4.CreateScale(scale.v) *
				Matrix4x4.CreateFromQuaternion(rotation);
			result.M41 += translation.v.X;
			result.M42 += translation.v.Y;
			result.M43 += translation.v.Z;
			return result;
		}
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
			=> TRS(translation, Quat.FromAngles(pitchYawRollDeg), new Vec3(scale, scale, scale));
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
			=> TRS(translation, Quat.FromAngles(pitchYawRollDeg), scale);

		/// <summary>This creates a matrix used for projecting 3D geometry
		/// onto a 2D surface for rasterization. Perspective projection 
		/// matrices will cause parallel lines to converge at the horizon. 
		/// This is great for normal looking content.</summary>
		/// <param name="fovDegrees">This is the vertical field of view of
		/// the perspective matrix, units are in degrees.</param>
		/// <param name="aspectRatio">The projection surface's width/height.
		/// </param>
		/// <param name="nearClip">Anything closer than this distance (in
		/// meters) will be discarded. Must not be zero, and if you make this
		/// too small, you may experience glitching in your depth buffer.</param>
		/// <param name="farClip">Anything further than this distance (in
		/// meters) will be discarded. For low resolution depth buffers, this
		/// should not be too far away, or you'll see bad z-fighting 
		/// artifacts.</param>
		/// <returns>The final perspective matrix.</returns>
		public static Matrix Perspective(float fovDegrees, float aspectRatio, float nearClip, float farClip)
			=> Matrix4x4.CreatePerspectiveFieldOfView(fovDegrees*Units.deg2rad, aspectRatio, nearClip, farClip);

		/// <summary>A transformation that describes one position looking at
		/// another point. This is particularly useful for describing camera
		/// transforms!</summary>
		/// <param name="from">The location the transform is looking from, or
		/// the position of the 'camera'.</param>
		/// <param name="at">The location the transform is looking towards, or
		/// the subject of the 'camera'.</param>
		/// <returns>A common camera-like transform matrix.</returns>
		public static Matrix LookAt(Vec3 from, Vec3 at)
			=> Matrix4x4.CreateLookAt(from, at, Vec3.Up);

		/// <inheritdoc cref="LookAt(Vec3, Vec3)"/>
		/// <param name="up">This controlls the roll value of the lookat
		/// transform, this would be the direction the top of the camera is
		/// facing. In most cases, this is just Vec3.Up.</param>
		public static Matrix LookAt(Vec3 from, Vec3 at, Vec3 up)
			=> Matrix4x4.CreateLookAt(from, at, up);

		/// <summary>This creates a matrix used for projecting 3D geometry
		/// onto a 2D surface for rasterization. With the known camera 
		/// intrinsics, you can replicate its perspective!</summary>
		/// <param name="imageResolution">The resolution of the image. This
		/// should be the image's width and height in pixels.</param>
		/// <param name="focalLengthPx">The focal length of camera in pixels,
		/// with image coordinates +X (pointing right) and +Y (pointing up).</param>
		/// <param name="nearClip">Anything closer than this distance (in
		/// meters) will be discarded. Must not be zero, and if you make this
		/// too small, you may experience glitching in your depth buffer.</param>
		/// <param name="farClip">Anything further than this distance (in
		/// meters) will be discarded. For low resolution depth buffers, this
		/// should not be too far away, or you'll see bad z-fighting 
		/// artifacts.</param>
		/// <returns>The final perspective matrix.</returns>
		/// <remarks>Think of the optical axis as an imaginary line that passes through
		/// the camera lens. In front of the camera lens, there's an image plane, 
		/// perpendicular to the optical axis, where the image of the scene being 
		/// captured is formed. Its distance is equal to the focal length of the camera 
		/// from the center of the lens. Here, we find the ratio between the size of 
		/// the image plane and distance from the camera in one unit distance and multiply 
		/// it by the near clip distance to find a near plane that is parallel.</remarks>
		public static Matrix Perspective(Vec2 imageResolution, Vec2 focalLengthPx, float nearClip, float farClip)
		{
			Vec2 nearPlaneDimensions = imageResolution / focalLengthPx * nearClip;
			return Matrix4x4.CreatePerspective(nearPlaneDimensions.x, nearPlaneDimensions.y, nearClip, farClip);
		}
		
		/// <summary>This creates a matrix used for projecting 3D geometry
		/// onto a 2D surface for rasterization. With the known camera 
		/// intrinsics, you can replicate its perspective!</summary>
		/// <param name="imageResolution">The resolution of the image. This
		/// should be the image's width and height in pixels.</param>
		/// <param name="focalLengthPx">The focal length of the camera in pixels,
		/// with image coordinates +X (pointing right) and +Y (pointing up).</param>
		/// <param name="principalPointPx">The principal point of the camera in pixels,
		/// with image coordinates +X (pointing right) and +Y (pointing up).</param>
		/// <param name="nearClip">Anything closer than this distance (in
		/// meters) will be discarded. Must not be zero, and if you make this
		/// too small, you may experience glitching in your depth buffer.</param>
		/// <param name="farClip">Anything further than this distance (in
		/// meters) will be discarded. For low resolution depth buffers, this
		/// should not be too far away, or you'll see bad z-fighting 
		/// artifacts.</param>
		/// <returns>The final perspective matrix.</returns>
		/// <remarks>The principal point is usually close to the center of the image
		/// but there may be an offset due to various factors, like lens misalignment
		/// or manufacturing imperfections. Think of the optical axis as an imaginary line
		/// that passes through the camera lens. In front of the camera lens, there's an 
		/// image plane, perpendicular to the optical axis, where the image of the scene 
		/// being captured is formed. Its distance is equal to the focal length of the
		/// camera from the center of the lens. The principal point is the point on the 
		/// image plane where the optical axis intersects it, which may be offset from
		/// the center of the image. Here, we calculate the left, right, bottom, and top
		/// values of the view frustum from the principal point on the near plane.</remarks>
		public static Matrix Perspective(Vec2 imageResolution, Vec2 focalLengthPx, Vec2 principalPointPx, float nearClip, float farClip)
		{
			float l = nearClip / focalLengthPx.x * -principalPointPx.x;
			float r = nearClip / focalLengthPx.x * (imageResolution.x - principalPointPx.x);
			float b = nearClip / focalLengthPx.y * (principalPointPx.y - imageResolution.y);
			float t = nearClip / focalLengthPx.y * principalPointPx.y;
			return Matrix4x4.CreatePerspectiveOffCenter(l, r, b, t, nearClip, farClip);
		}

		/// <summary>This creates a matrix used for projecting 3D geometry
		/// onto a 2D surface for rasterization. Orthographic projection 
		/// matrices will preserve parallel lines. This is great for 2D 
		/// scenes or content.</summary>
		/// <param name="width">The width, in meters, of the area that will 
		/// be projected.</param>
		/// <param name="height">The height, in meters, of the area that will
		/// be projected.</param>
		/// <param name="nearClip">Anything closer than this distance (in
		/// meters) will be discarded. Must not be zero, and if you make this
		/// too small, you may experience glitching in your depth buffer.</param>
		/// <param name="farClip">Anything further than this distance (in
		/// meters) will be discarded. For low resolution depth buffers, this
		/// should not be too far away, or you'll see bad z-fighting 
		/// artifacts.</param>
		/// <returns>The final orthographic matrix.</returns>
		public static Matrix Orthographic(float width, float height, float nearClip, float farClip)
			=> Matrix4x4.CreateOrthographic(width, height, nearClip, farClip);
	}
}
