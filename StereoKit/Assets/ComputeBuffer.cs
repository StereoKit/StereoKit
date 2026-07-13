using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>A GPU storage buffer for shuttling data to and from
	/// compute shaders! In HLSL, this maps to StructuredBuffer&lt;T&gt;
	/// or RWStructuredBuffer&lt;T&gt; depending on the
	/// ComputeBufferType.
	///
	/// Your struct T needs to be unmanaged (no reference types!) and
	/// its layout _must_ match the HLSL struct. Watch out for
	/// padding differences between C# and HLSL!</summary>
	/// <typeparam name="T">An unmanaged struct matching the HLSL
	/// buffer element layout.</typeparam>
	public class ComputeBuffer<T> : IAsset where T : unmanaged
	{
		internal IntPtr _inst;

		/// <summary>Gets or sets the unique identifier of this asset
		/// resource! This can be helpful for debugging, managing your
		/// assets, or finding them later on!</summary>
		public string Id
		{
			get => Marshal.PtrToStringAnsi(NativeAPI.compute_buffer_get_id(_inst));
			set => NativeAPI.compute_buffer_set_id(_inst, value);
		}

		/// <summary>The capacity of the buffer, in number of T elements.
		/// This is the max you can Set or Get without clamping.
		/// </summary>
		public int Count => NativeAPI.compute_buffer_get_count(_inst);

		/// <summary>Size of a single element in bytes, derived from
		/// your T struct. Handy for sanity-checking that your C# struct
		/// matches the HLSL side!</summary>
		public int Stride => NativeAPI.compute_buffer_get_stride(_inst);

		/// <summary>Creates a GPU storage buffer with room for
		/// elementCount elements, initially uninitialized! The contents
		/// will be whatever was in GPU memory before, so make sure to
		/// write before you read.</summary>
		/// <param name="type">Read or ReadWrite access from compute
		/// shaders.</param>
		/// <param name="elementCount">Number of T elements to
		/// allocate.</param>
		public ComputeBuffer(ComputeBufferType type, int elementCount)
		{
			int stride = Marshal.SizeOf<T>();
			_inst = NativeAPI.compute_buffer_create(type, elementCount, stride, IntPtr.Zero);
			if (_inst == IntPtr.Zero)
				Log.Err("Failed to create ComputeBuffer!");
		}

		/// <summary>Creates a GPU storage buffer and immediately uploads
		/// initialData to it! The buffer capacity is set to the array
		/// length.</summary>
		/// <param name="type">Read or ReadWrite access from compute
		/// shaders.</param>
		/// <param name="initialData">Array of data to upload to the
		/// GPU.</param>
		public ComputeBuffer(ComputeBufferType type, T[] initialData)
		{
			int stride = Marshal.SizeOf<T>();
			GCHandle handle = GCHandle.Alloc(initialData, GCHandleType.Pinned);
			try
			{
				_inst = NativeAPI.compute_buffer_create(type, initialData.Length, stride, handle.AddrOfPinnedObject());
			}
			finally
			{
				handle.Free();
			}
			if (_inst == IntPtr.Zero)
				Log.Err("Failed to create ComputeBuffer!");
		}

		/// <summary>Release reference to the StereoKit asset.</summary>
		~ComputeBuffer()
		{
			if (_inst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe(_inst);
		}

		/// <summary>Upload data from the CPU to the GPU! If data.Length
		/// exceeds the buffer's capacity, it will be clamped with a
		/// warning.</summary>
		/// <param name="data">The data to upload.</param>
		public void SetData(T[] data)
		{
			GCHandle handle = GCHandle.Alloc(data, GCHandleType.Pinned);
			try
			{
				NativeAPI.compute_buffer_set_data(_inst, handle.AddrOfPinnedObject(), data.Length);
			}
			finally
			{
				handle.Free();
			}
		}

		/// <summary>Read the full buffer back from the GPU! This blocks
		/// until the data is ready, and allocates a new array each
		/// call. For per-frame readbacks, prefer the GetData(ref T[])
		/// overload to avoid GC pressure!</summary>
		/// <returns>A new array containing the full buffer contents.
		/// </returns>
		public T[] GetData()
		{
			int      count  = Count;
			T[]      data   = new T[count];
			GCHandle handle = GCHandle.Alloc(data, GCHandleType.Pinned);
			try
			{
				NativeAPI.compute_buffer_get_data(_inst, handle.AddrOfPinnedObject(), count);
			}
			finally
			{
				handle.Free();
			}
			return data;
		}

		/// <summary>Read GPU data into a pre-allocated array! This is
		/// the allocation-free version of GetData, great for calling
		/// every frame without creating GC garbage. Reads
		/// Math.Min(data.Length, Count) elements.</summary>
		/// <param name="data">A pre-allocated array to fill. If it's
		/// smaller than the buffer, only data.Length elements are
		/// read.</param>
		public void GetData(ref T[] data)
		{
			int count = Math.Min(data.Length, Count);
			GCHandle handle = GCHandle.Alloc(data, GCHandleType.Pinned);
			try
			{
				NativeAPI.compute_buffer_get_data(_inst, handle.AddrOfPinnedObject(), count);
			}
			finally
			{
				handle.Free();
			}
		}
	}
}
