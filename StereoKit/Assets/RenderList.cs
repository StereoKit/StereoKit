using System;

namespace StereoKit
{
	/// <summary>A RenderList is a collection of Draw commands that can be
	/// submitted to various surfaces. RenderList.Primary is where all normal
	/// Draw calls get added to, and this RenderList is renderer to primary
	/// display surface.
	/// 
	/// Manually working with a RenderList can be useful for "baking down
	/// matrices" or caching a scene of objects. Or for drawing a separate
	/// scene to an offscreen surface, like for thumbnails of Models.</summary>
	public class RenderList
	{
		internal IntPtr _inst;

		/// <summary>The number of Mesh/Material pairs that have been submitted
		/// to the render list so far this frame.</summary>
		public int Count => NativeAPI.render_list_item_count(_inst);
		/// <summary>This is the number of items in the RenderList before it
		/// was most recently cleared. If this is a list that is drawn and
		/// cleared each frame, you can think of this as "last frame's count".
		/// </summary>
		public int PrevCount => NativeAPI.render_list_prev_count(_inst);

		/// <summary>Creates a new empty RenderList.</summary>
		public RenderList()
		{
			_inst = NativeAPI.render_list_create();
		}
		internal RenderList(IntPtr material)
		{
			_inst = material;
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty RenderList!");
		}
		/// <summary>Release reference to the StereoKit asset.</summary>
		~RenderList()
		{
			if (_inst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe(_inst);
		}

		/// <summary>Clears out and de-references all Draw items currently in
		/// the RenderList.</summary>
		public void Clear() => NativeAPI.render_list_clear(_inst);

		public void Add(Mesh mesh, Material material, Matrix worldTransform, Color colorLinear, RenderLayer layer = RenderLayer.Layer0)
			=> NativeAPI.render_list_add_mesh(_inst, mesh._inst, material._inst, worldTransform, colorLinear, layer);

		public void Add(Model model, Matrix worldTransform, Color colorLinear, RenderLayer layer = RenderLayer.Layer0)
			=> NativeAPI.render_list_add_model(_inst, model._inst, worldTransform, colorLinear, layer);

		public void Add(Model model, Material materialOverride, Matrix worldTransform, Color colorLinear, RenderLayer layer = RenderLayer.Layer0)
			=> NativeAPI.render_list_add_model_mat(_inst, model._inst, materialOverride._inst, worldTransform, colorLinear, layer);

		public void DrawNow(Tex toRenderTarget, Matrix camera, Matrix projection, Rect viewport, RenderLayer layerFilter = RenderLayer.All, RenderClear clear = RenderClear.All)
			=> NativeAPI.render_list_draw_now(_inst, toRenderTarget._inst, camera, projection, viewport, layerFilter, clear);

		/// <summary>The default RenderList used by the Renderer for the
		/// primary display surface.</summary>
		public static RenderList Primary => Default.RenderList;

		public static void Push(RenderList list) => NativeAPI.render_list_push(list._inst);
		public static void Pop() => NativeAPI.render_list_pop();
	}
}
