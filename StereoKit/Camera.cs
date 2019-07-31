using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    internal struct camera_t
    {
        float fov;
        float clip_near;
        float clip_far;
    }

    public class Camera
    {
        #region Imports
        [DllImport(NativeLib.DllName)] static extern void camera_initialize(IntPtr transform, float fov, float clip_near, float clip_far);
        #endregion

        internal IntPtr _cameraInst;
        public Camera(float fov, float clipNear, float clipFar)
        {
            _cameraInst = Marshal.AllocCoTaskMem(Marshal.SizeOf(typeof(camera_t)));
            camera_initialize(_cameraInst, fov, clipNear, clipFar);
        }
        ~Camera()
        {
            if (_cameraInst != IntPtr.Zero)
                Marshal.FreeCoTaskMem(_cameraInst);
        }
    }
}
