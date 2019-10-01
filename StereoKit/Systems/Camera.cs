using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    public class Camera
    {
        internal IntPtr _cameraInst;
        public Camera(float fov, float clipNear, float clipFar)
        {
            _cameraInst = Marshal.AllocCoTaskMem(Marshal.SizeOf(typeof(camera_t)));
            NativeAPI.camera_initialize(_cameraInst, fov, clipNear, clipFar);
        }
        ~Camera()
        {
            if (_cameraInst != IntPtr.Zero)
                Marshal.FreeCoTaskMem(_cameraInst);
        }
    }
}
