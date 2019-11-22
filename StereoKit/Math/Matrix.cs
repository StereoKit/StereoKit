using System.Runtime.InteropServices;

namespace StereoKit
{
    /// <summary>A Matrix in StereoKit is a 4x4 grid of numbers that is used to represent
    /// a transformation for any sort of position or vector! This is an oversimplification
    /// of what a matrix actually is, but it's accurate in this case.
    /// 
    /// Matrices are really useful for transforms because you can chain together all sorts
    /// of transforms into a single Matrix! A Matrix transform really shines when applied to
    /// many positions, as the more expensive operations get cached within the matrix values.
    /// 
    /// Matrices are prominently used within shaders for mesh transforms!</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Matrix
    {
        public Vec4 row1;
        public Vec4 row2;
        public Vec4 row3;
        public Vec4 row4;

        /// <summary>Creates an inverse matrix! If the matrix takes a point from a -> b, then
        /// its inverse takes the point from b -> a.</summary>
        /// <returns>An inverse matrix of the current one.</returns>
        public Matrix Inverse() 
        {
            NativeAPI.matrix_inverse(this, out Matrix result); 
            return result;
        }
        /// <summary>Transforms a point through the Matrix! This is basically just multiplying
        /// a vector (x,y,z,1) with the Matrix.</summary>
        /// <param name="point">The point to transform.</param>
        /// <returns>The point transformed by the Matrix.</returns>
        public Vec3 TransformPoint    (Vec3 point)     => NativeAPI.matrix_mul_point    (this, point);
        /// <summary> Transforms a point through the Matrix, but excluding translation! This is great
        /// for transforming vectors that are -directions- rather than points in space. Use this to
        /// transform normals and directions. The same as multiplying (x,y,z,0) with the Matrix.</summary>
        /// <param name="direction">The direction to transform.</param>
        /// <returns>The direction transformed by the Matrix.</returns>
        public Vec3 TransformDirection(Vec3 direction) => NativeAPI.matrix_mul_direction(this, direction);

        public static Matrix operator *(Matrix a, Matrix b) { 
            NativeAPI.matrix_mul(a, b, out Matrix result);
            return result;
        }
        public static Vec3   operator *(Matrix a, Vec3 b) => NativeAPI.matrix_mul_point(a, b);

        /// <summary>Translate, Rotate, Scale. Creates a transform Matrix using all these components!</summary>
        /// <param name="translation">Move an object by this amount.</param>
        /// <param name="rotation">A Quaternion describing the rotation for this transform.</param>
        /// <param name="scale">How much larger or smaller this transform makes things. 1 is a good
        /// default, as 0 will shrink it to nothing! This will expand to a scale vector of (size, size, size)</param>
        /// <returns>A Matrix that combines translation, roatation, and scale information into a single Matrix!</returns>
        public static Matrix TRS(Vec3 translation, Quat rotation, float scale=1) => NativeAPI.matrix_trs(translation, rotation, new Vec3(scale, scale, scale));

        /// <summary>Translate, Rotate, Scale. Creates a transform Matrix using all these components!</summary>
        /// <param name="translation">Move an object by this amount.</param>
        /// <param name="rotation">A Quaternion describing the rotation for this transform.</param>
        /// <param name="scale">How much larger or smaller this transform makes things. Vec3.One is a good
        /// default, as Vec3.Zero will shrink it to nothing!</param>
        /// <returns>A Matrix that combines translation, roatation, and scale information into a single Matrix!</returns>
        public static Matrix TRS(Vec3 translation, Quat rotation, Vec3 scale) => NativeAPI.matrix_trs(translation, rotation, scale);

        /// <summary>Translate, Scale. Creates a transform Matrix using both these components!</summary>
        /// <param name="translation">Move an object by this amount.</param>
        /// <param name="scale">How much larger or smaller this transform makes things. Vec3.One is a good
        /// default, as Vec3.Zero will shrink it to nothing!</param>
        /// <returns>A Matrix that combines translation and scale information into a single Matrix!</returns>
        public static Matrix TS(Vec3 translation, Vec3 scale) => NativeAPI.matrix_trs(translation, Quat.Identity, scale);

        /// <summary>Translate, Scale. Creates a transform Matrix using both these components!</summary>
        /// <param name="translation">Move an object by this amount.</param>
        /// <param name="scale">How much larger or smaller this transform makes things. 1 is a good
        /// default, as 0 will shrink it to nothing! This will expand to a scale vector of (size, size, size)</param>
        /// <returns>A Matrix that combines translation and scale information into a single Matrix!</returns>
        public static Matrix TS(Vec3 translation, float scale=1) => NativeAPI.matrix_trs(translation, Quat.Identity, new Vec3(scale, scale, scale));

        /// <summary>Translate. Creates a translation Matrix!</summary>
        /// <param name="translation">Move an object by this amount.</param>
        /// <returns>A Matrix containing a simple translation!</returns>
        public static Matrix T(Vec3 translation) => NativeAPI.matrix_trs(translation, Quat.Identity, Vec3.One);

        /// <summary>Translate. Creates a translation Matrix!</summary>
        /// <param name="x">Move an object on the x axis by this amount.</param>
        /// <param name="y">Move an object on the y axis by this amount.</param>
        /// <param name="z">Move an object on the z axis by this amount.</param>
        /// <returns>A Matrix containing a simple translation!</returns>
        public static Matrix T(float x, float y, float z) => NativeAPI.matrix_trs(new Vec3(x,y,z), Quat.Identity, Vec3.One);


        /// <summary>An identity Matrix is the matrix equivalent of '1'! Transforming anything by this
        /// will leave it at the exact same place.</summary>
        public static Matrix Identity { get{
            return new Matrix { 
                row1 = new Vec4(1,0,0,0),
                row2 = new Vec4(0,1,0,0),
                row3 = new Vec4(0,0,1,0),
                row4 = new Vec4(0,0,0,1),
            };
        } }
    }
}
