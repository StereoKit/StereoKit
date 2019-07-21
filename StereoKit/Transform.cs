using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vec3
    {
        public float x, y, z;

        public Vec3(float aX, float aY, float aZ)
        {
            x = aX;
            y = aY;
            z = aZ;
        }

        public static readonly Vec3 Zero = new Vec3(0,0,0);
        public static readonly Vec3 One  = new Vec3(1,1,1);
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct Vec4
    {
        public float x, y, z, w;
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct Quat
    {
        public float i, j, k, a;

        public Quat(float i, float j, float k, float a)
        {
            this.i = i;
            this.j = j;
            this.k = k;
            this.a = a;
        }

        public static readonly Quat Identity = new Quat(0, 0, 0, 1);
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct Matrix
    {
        public Vec4 row1;
        public Vec4 row2;
        public Vec4 row3;
        public Vec4 row4;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct transform_t
    {
        Vec3 _position;
        Vec3 _scale;
        Quat _rotation;

        bool _dirty;
        Matrix _transform;
    }

    public class Transform
    {
        #region Imports
        [DllImport(Util.DllName)] static extern void transform_initialize(IntPtr transform);
        [DllImport(Util.DllName)] static extern void transform_set       (IntPtr transform, Vec3 position, Vec3 scale, Quat rotation );
        [DllImport(Util.DllName)] static extern void transform_set_pos   (IntPtr transform, Vec3 position);
        [DllImport(Util.DllName)] static extern void transform_set_scale (IntPtr transform, Vec3 scale);
        [DllImport(Util.DllName)] static extern void transform_set_rot   (IntPtr transform, Quat rotation);
        [DllImport(Util.DllName)] static extern void transform_lookat    (IntPtr transform, Vec3 at);
        #endregion

        internal IntPtr _transformInst;
        public Transform()
        {
            _transformInst = Marshal.AllocCoTaskMem(Marshal.SizeOf<transform_t>());
            transform_initialize(_transformInst);
        }
        public Transform(Vec3 position, Vec3? scale = null, Quat? rotation = null)
        {
            _transformInst = Marshal.AllocCoTaskMem(Marshal.SizeOf<transform_t>());
            transform_initialize(_transformInst);
            transform_set(_transformInst, position, scale.HasValue ? scale.Value : Vec3.One, rotation.HasValue ? rotation.Value : Quat.Identity);
        }
        ~Transform()
        {
            if (_transformInst != IntPtr.Zero)
                Marshal.FreeCoTaskMem(_transformInst);
        }

        public Vec3 Pos   { set { transform_set_pos  (_transformInst, value); } }
        public Quat Rot   { set { transform_set_rot  (_transformInst, value); } }
        public Vec3 scale { set { transform_set_scale(_transformInst, value); } }

        public void LookAt(Vec3 pos) { transform_lookat(_transformInst, pos); }
    }
}
