using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>This is a chunk of memory that will get bound to all shaders
	/// at a particular register slot. StereoKit uses this to provide engine
	/// values to the shader, and you can also use this to drive graphical
	/// shader systems of your own! 
	/// 
	/// For example, if your application has a custom lighting system, fog,
	/// wind, or some other system that multiple shaders might need to refer
	/// to, this is the perfect tool to use.
	/// 
	/// The type 'T' for this buffer must be a struct that uses the 
	/// `[StructLayout(LayoutKind.Sequential)]` attribute for proper copying.
	/// It should also match the layout of your equivalent cbuffer in the 
	/// shader file. Note that shaders often have [specific byte alignment](https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules) 
	/// requirements! Example:
	/// 
	/// C#
	/// ```csharp
	///[StructLayout(LayoutKind.Sequential)]
	///struct BufferData {
	///	   Vec3  windDirection;
	///	   float windStrength
	///}
	/// ```
	/// 
	/// HLSL
	/// ```c
	///cbuffer BufferData : register(b3) {
	///	   float3 windDirection;
	///	   float  windStrength;
	///}
	/// ```
	/// </summary>
	/// <typeparam name="T">a struct that uses the 
	/// `[StructLayout(LayoutKind.Sequential)]` attribute for proper copying
	/// </typeparam>
	public class MaterialBuffer<T> where T : unmanaged
	{
		internal IntPtr _inst;

		/// <summary>Create a new global MaterialBuffer bound to the register
		/// slot id. All shaders will have access to the data provided via 
		/// this instance's `Set`.</summary>
		/// <param name="registerSlot">Valid values are 3-16. This is the 
		/// register id that this data will be bound to. In HLSL, you'll see
		/// the slot id for '3' indicated like this `: register(b3)`</param>
		[Obsolete("Use empty constructor, and call Renderer.SetGlobalBuffer instead! This will be removed in a future version.")]
		public MaterialBuffer(int registerSlot) {
			int size = Marshal.SizeOf(typeof(T));
			_inst    = NativeAPI.material_buffer_create(size);
			if (_inst == IntPtr.Zero)
				throw new ArgumentException("Bad slot id, see log.");
			else
				Renderer.SetGlobalBuffer(registerSlot, this);
		}

		/// <summary>Create a new global MaterialBuffer that can be bound to a
		/// register slot id via Renderer.SetGlobalBuffer. All shaders will
		/// have access to the data provided via this instance's `Set`.</summary>
		public MaterialBuffer()
		{
			int size = Marshal.SizeOf(typeof(T));
			_inst    = NativeAPI.material_buffer_create(size);
		}

		/// <summary>Release reference to the StereoKit asset.</summary>
		~MaterialBuffer()
		{
			if (_inst != IntPtr.Zero)
				SK.ExecuteOnMain(() => NativeAPI.material_buffer_release(_inst));
		}

		/// <summary>This will upload your data to the GPU for shaders to use.
		/// </summary>
		/// <param name="data">The data you wish to upload to the GPU.</param>
		public void Set(in T data)
		{
			unsafe
			{
				fixed (void*dataPtr = &data) 
					NativeAPI.material_buffer_set_data(_inst, dataPtr);
			}
		}
	}
}
