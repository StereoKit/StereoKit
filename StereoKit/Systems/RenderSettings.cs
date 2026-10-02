using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>Optional settings for rendering a camera viewpoint to a
	/// rendertarget, used by Renderer.RenderTo and RenderList.DrawNow. This
	/// is a plain struct where zero means 'default' - a `default` or `new`
	/// RenderSettings gives you: all layers, the default material variant,
	/// clear everything to transparent black, a full-target viewport, and no
	/// post-processing.</summary>
	public struct RenderSettings
	{
		/// <summary>This is a bit flag that allows you to change which
		/// layers StereoKit renders for this particular pass. To change
		/// what layers a visual is on, use a Draw method that includes a
		/// RenderLayer as a parameter. A value of 0 (the default) means
		/// RenderLayer.All.</summary>
		public RenderLayer layerFilter;
		/// <summary>Specifies which Material variant should be used for
		/// rendering. 0 is the normal default material, any others will
		/// generally be application-defined by setting up each Material's
		/// Variant with specific shaders. If a Material has no corresponding
		/// variant, it will not be drawn.</summary>
		public int         materialVariant;
		/// <summary>Describes if and how the rendertarget should be cleared
		/// before rendering. A value of 0 (the default) clears everything,
		/// use RenderClear.Keep to draw on top of the target's existing
		/// content. Note that clearing the target is unaffected by the
		/// viewport, so this will clean the entire surface!</summary>
		public RenderClear clear;
		/// <summary>If `clear` clears color, this is the color it will
		/// clear to, in linear space. Default is a transparent black.</summary>
		public Color       clearColor;
		/// <summary>Allows you to specify a region of the rendertarget to
		/// draw to! This is in normalized coordinates, 0-1. If the width of
		/// this value is zero (the default), then this will render to the
		/// entire texture.</summary>
		public Rect        viewport;
		/// <summary>An optional list of post-process Materials for this
		/// pass, applied in array order. These are tile-friendly subpass
		/// effects, see Renderer.SetPostProcess for the shader
		/// requirements. Null is fine here, and means no
		/// post-processing.</summary>
		public Material[]  postProcess;

		/// <summary>Marshals to the native render_settings_t. When a
		/// post-process list is present, the returned pin holds the
		/// material pointer array and must be freed after the native call
		/// completes.</summary>
		internal RenderSettingsNative ToNative(out GCHandle pin)
		{
			pin = default;
			RenderSettingsNative native = new RenderSettingsNative {
				layerFilter     = layerFilter,
				materialVariant = materialVariant,
				clear           = clear,
				clearColor      = clearColor,
				viewport        = viewport,
			};
			if (postProcess != null && postProcess.Length > 0)
			{
				IntPtr[] materials = new IntPtr[postProcess.Length];
				for (int i = 0; i < materials.Length; i++)
					materials[i] = postProcess[i]?._inst ?? IntPtr.Zero;
				pin = GCHandle.Alloc(materials, GCHandleType.Pinned);
				native.postProcess      = pin.AddrOfPinnedObject();
				native.postProcessCount = materials.Length;
			}
			return native;
		}
	}

	/// <summary>Blittable mirror of the native render_settings_t.</summary>
	[StructLayout(LayoutKind.Sequential)]
	internal struct RenderSettingsNative
	{
		public RenderLayer layerFilter;
		public int         materialVariant;
		public RenderClear clear;
		public Color       clearColor;
		public Rect        viewport;
		public IntPtr      postProcess;
		public int         postProcessCount;
	}
}
