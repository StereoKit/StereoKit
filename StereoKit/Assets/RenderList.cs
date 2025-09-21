using System;
using System.Runtime.InteropServices;

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
	public class RenderList : IAsset
	{
		internal IntPtr _inst;

		/// <summary>Gets or sets the unique identifier of this asset resource!
		/// This can be helpful for debugging, managing your assets, or finding
		/// them later on!</summary>
		public string Id
		{
			get { unsafe { return NU8.Str(NativeAPI.render_list_get_id(_inst)); } }
			set { unsafe { byte* val = NU8.Bytes(value); NativeAPI.render_list_set_id(_inst, val); NU8.Free(val); } }
		}

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
		internal RenderList(IntPtr list)
		{
			_inst = list;
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty RenderList!");
		}
		/// <summary>Release reference to the StereoKit asset.</summary>
		~RenderList()
		{ unsafe {
			if (_inst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe((void*)_inst);
		} }

		/// <summary>Clears out and de-references all Draw items currently in
		/// the RenderList.</summary>
		public void Clear() => NativeAPI.render_list_clear(_inst);

		/// <summary>Add a Mesh/Material to the RenderList. The RenderList will
		/// hold a reference to these Assets until the list is cleared.</summary>
		/// <param name="mesh">A valid Mesh you wish to draw.</param>
		/// <param name="material">A Material to apply to the Mesh.</param>
		/// <param name="transform">A transformation Matrix relative to the
		/// current Hierarchy.</param>
		/// <param name="colorLinear">A per-instance linear space color value
		/// to pass into the shader! Normally this gets used like a material
		/// tint. If you're  adventurous and don't need per-instance colors,
		/// this is a great spot to pack in extra per-instance data for the
		/// shader!</param>
		/// <param name="layer">All visuals are rendered using a layer
		/// bit-flag. By default, all layers are rendered, but this can be
		/// useful for filtering out objects for different rendering
		/// purposes! For example: rendering a mesh over the user's head from
		/// a 3rd person perspective, but filtering it out from the 1st
		/// person perspective.</param>
		public void Add(Mesh mesh, Material material, Matrix transform, Color colorLinear, RenderLayer layer = RenderLayer.Layer0)
			=> NativeAPI.render_list_add_mesh(_inst, mesh._inst, material._inst, transform, colorLinear, layer);

		/// <summary>Add a Model to the RenderList. The RenderList will
		/// hold a reference to these Assets until the list is cleared.</summary>
		/// <param name="model">A valid Model you wish to draw.</param>
		/// <param name="transform">A transformation Matrix relative to the
		/// current Hierarchy.</param>
		/// <param name="colorLinear">A per-instance linear space color value
		/// to pass into the shader! Normally this gets used like a material
		/// tint. If you're  adventurous and don't need per-instance colors,
		/// this is a great spot to pack in extra per-instance data for the
		/// shader!</param>
		/// <param name="layer">All visuals are rendered using a layer
		/// bit-flag. By default, all layers are rendered, but this can be
		/// useful for filtering out objects for different rendering
		/// purposes! For example: rendering a mesh over the user's head from
		/// a 3rd person perspective, but filtering it out from the 1st
		/// person perspective.</param>
		public void Add(Model model, Matrix transform, Color colorLinear, RenderLayer layer = RenderLayer.Layer0)
			=> NativeAPI.render_list_add_model(_inst, model._inst, transform, colorLinear, layer);

		/// <inheritdoc cref="Add(Model, Matrix, Color, RenderLayer)" />
		/// <param name="materialOverride">Allows you to override the Material
		/// of all nodes on this Model with your own Material.</param>
		public void Add(Model model, Material materialOverride, Matrix transform, Color colorLinear, RenderLayer layer = RenderLayer.Layer0)
			=> NativeAPI.render_list_add_model_mat(_inst, model._inst, materialOverride._inst, transform, colorLinear, layer);

		/// <summary>Draws the RenderList to a rendertarget texture
		/// immediately. It does _not_ clear the list.</summary>
		/// <param name="toRenderTarget">The rendertarget texture to draw to.
		/// </param>
		/// <param name="camera">A TRS matrix representing the location and
		/// orientation of the camera. This matrix gets inverted later on, so
		/// no need to do it yourself.</param>
		/// <param name="projection">The projection matrix describes how the
		/// geometry is flattened onto the draw surface. Normally, you'd use
		/// Matrix.Perspective, and occasionally Matrix.Orthographic might be
		/// helpful as well.</param>
		/// <param name="clearColor">If the `clear` parameter is set to clear
		/// the color of `toRenderTarget`, then this is the color it will clear
		/// to. `default` would be a transparent black.</param>
		/// <param name="clear">Describes if and how the rendertarget should
		/// be cleared before rendering. Note that clearing the target is
		/// unaffected by the viewport, so this will clean the entire
		/// surface!</param>
		/// <param name="viewportPct">Allows you to specify a region of the
		/// rendertarget to draw to! This is in normalized coordinates, 0-1.
		/// If the width of this value is zero, then this will render to the
		/// entire texture.</param>
		/// <param name="layerFilter">This is a bit flag that allows you to
		/// change which layers StereoKit renders for this particular render
		/// viewpoint. To change what layers a visual is on, use a Draw
		/// method that includes a RenderLayer as a parameter.</param>
		/// <param name="materialVariant">Specifies which Material variant
		/// should be used for rendering. 0 will be the normal default
		/// material, any others will generally be application-defined by
		/// setting up each Material's Variant with specific shaders. If a
		/// Material has no corresponding variant, it will not be drawn.
		/// </param>
		public void DrawNow(Tex toRenderTarget, Matrix camera, Matrix projection, Color clearColor = default, RenderClear clear = RenderClear.All, Rect viewportPct = default, RenderLayer layerFilter = RenderLayer.All, int materialVariant = 0)
			=> NativeAPI.render_list_draw_now(_inst, toRenderTarget._inst, camera, projection, clearColor, clear, viewportPct, layerFilter, materialVariant);

		/// <summary>The default RenderList used by the Renderer for the
		/// primary display surface.</summary>
		public static RenderList Primary => Default.RenderList;

		/// <summary>All draw calls that don't specify a render list will get
		/// submitted to the active RenderList at the top of the stack. By
		/// default, that's RenderList.Primary, but you can push your own list
		/// onto the stack here to capture draw calls, like those done in the
		/// UI.</summary>
		/// <param name="list">The list that should go on top of the stack as
		/// the active RenderList.</param>
		public static void Push(RenderList list) => NativeAPI.render_list_push(list._inst);

		/// <summary>This removes the current top of the RenderList stack,
		/// making the next list as active</summary>
		public static void Pop() => NativeAPI.render_list_pop();

		/// <summary>Finds the RenderList with the matching id, and returns a
		/// reference to it. If no RenderList is found, it returns null.
		/// </summary>
		/// <param name="listId">Id of the RenderList we're looking for.
		/// </param>
		/// <returns>A RenderList with a matching id, or null if none is found.
		/// </returns>
		public static RenderList Find(string listId)
		{ unsafe {
			byte*  listIdBytes = NU8.Bytes(listId);
			IntPtr list        = NativeAPI.render_list_find(listIdBytes);
			NU8.Free(listIdBytes);
			return list == IntPtr.Zero ? null : new RenderList(list);
		} }
	}
}