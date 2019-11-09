using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Matrix
    {
        public Vec4 row1;
        public Vec4 row2;
        public Vec4 row3;
        public Vec4 row4;

        public Matrix Inverse() 
        {
            Matrix result; 
            NativeAPI.matrix_inverse(this, out result); 
            return result;
        }
        public Vec3 TransformPoint    (Vec3 point)     => NativeAPI.matrix_mul_point    (this, point);
        public Vec3 TransformDirection(Vec3 direction) => NativeAPI.matrix_mul_direction(this, direction);

        public static Matrix operator *(Matrix a, Matrix b) { 
            Matrix result;
            NativeAPI.matrix_mul(a, b, out result);
            return result;
        }
        public static Vec3   operator *(Matrix a, Vec3 b) => NativeAPI.matrix_mul_point(a, b);

        public static Matrix TRS(Vec3 position, Quat rotation, Vec3 scale) => NativeAPI.matrix_trs(position, rotation, scale);
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
