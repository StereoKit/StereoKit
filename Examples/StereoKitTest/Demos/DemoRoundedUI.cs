// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

using StereoKit;

internal class DemoRoundedUI : ITest
{
	string title       = "Rounded UI";
	string description = "";

	Pose winPose = Pose.Identity;

	RoundedSprite[] gallery = new RoundedSprite[] {
		new RoundedSprite("floor.png"),
		new RoundedSprite("metal_plate_diff.jpg"),
		new RoundedSprite("metal_plate_metal.jpg"),
		new RoundedSprite("test.png"),
		new RoundedSprite("test_normal.png"),
		new RoundedSprite("UVTex.png"),
	};

	public void Step()
	{
		UI.WindowBegin(title, ref winPose, new Vec2(0.22f,0));

		// Draw each RoundedSprite as a pressable UI button
		UI.Label("Interactive Buttons");
		for (int i = 0; i < gallery.Length; i++)
		{
			UIRoundedButton($"{i}", UI.LineHeight*2, gallery[i]);
			UI.SameLine();
		}

		UI.NextLine();
		UI.HSeparator();

		// Draw each RoundedSprite as a decorative UI image
		UI.Label("Static Images");
		for (int i = 0; i < gallery.Length; i++)
		{
			UIRoundedImage(UI.LineHeight * 2, gallery[i]);
			UI.SameLine();
		}
		UI.WindowEnd();

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0, -0.18f), V.XYZ(.3f, .45f, 0)));
	}

	public void Initialize() => winPose = (Demo.contentPose * winPose.ToMatrix()).Pose;
	public void Shutdown  () => RoundedSprite.ClearMesh();

	// Custom UI elements based on RoundedSprites

	static void UIRoundedImage(float height, RoundedSprite roundedSprite)
	{
		Vec2   size   = new Vec2(roundedSprite.aspect * height, height);
		Bounds layout = UI.LayoutReserve(size);

		UISettings settings = UI.Settings;
		layout.center    .z -= settings.depth / 2;
		layout.dimensions.z  = settings.depth;
		RoundedSprite.Mesh(settings.rounding).Draw(roundedSprite.material, Matrix.TS(layout.center, layout.dimensions), Color.White);
	}

	static bool UIRoundedButton(string idText, float height, RoundedSprite roundedSprite)
	{
		ulong id = UI.StackHash(idText);

		Vec2   size   = new Vec2(roundedSprite.aspect * height, height);
		Bounds layout = UI.LayoutReserve(size);
		UI.ButtonBehavior(layout.TLC, size, idText, out float offset, out BtnState state, out BtnState focus, out _);

		layout.center    .z -= offset / 2;
		layout.dimensions.z  = offset;
		RoundedSprite.Mesh(UI.Settings.rounding).Draw(roundedSprite.material, Matrix.TS(layout.center, layout.dimensions), UI.GetElementColor(UIVisual.Aura, UI.GetAnimFocus(id, focus, state)));

		return state.IsJustInactive();
	}

	/// <summary>A helper class for managing sprite-like textures with a
	/// rounded edge material. This class is DEPENDANT on the
	/// "Shaders/quadrant_textured.hlsl" shader in the /Examples/Assets/ folder
	/// </summary>
	class RoundedSprite
	{
		public Material material;
		public Tex      tex;
		public float    aspect;

		public RoundedSprite(string file)
		{
			tex             = Tex.FromFile(file);
			tex.AddressMode = TexAddress.Clamp;
			aspect          = 1;
			material        = new Material(_shaderTexturedQuadrant);
			material[MatParamName.DiffuseTex] = tex;
			// Prevent blocking when accessing Width / Height by waiting to
			// calculate the aspect until the texture is loaded.
			tex.OnLoaded += (t) => aspect = t.Width/(float)t.Height;
		}

		// Static on-demand asset management for reusability. These assets are
		// "common" assets used by all RoundedSprites, or RoundedSprite related
		// functions.
		private static Shader _shaderTexturedQuadrant = Shader.FromFile("Shaders/quadrant_textured.hlsl");
		private static Mesh   _roundedMesh            = null;
		private static float  _roundedRadius          = 0;
		/// <summary>This will get or create a Mesh you can use with
		/// RoundedSprites. It uses the corner radius provided to generate the
		/// Mesh, so if you ever present a new value, a new Mesh will be
		/// created. Because of this,if you plan to use a variety of radii, or
		/// even just more than one per-frame, you may need an alternative
		/// implementation.</summary>
		/// <param name="rounding">Mesh corner radius, in meters.</param>
		/// <returns>A quadrantified flat mesh with rounded corners.</returns>
		public static Mesh Mesh(float rounding)
		{
			// Create the rounded sprite mesh the first time, and recreate it
			// anytime the UI radius changes.
			if (rounding == _roundedRadius) return _roundedMesh;
			
			_roundedRadius = rounding;
			_roundedMesh   = UI.GenQuadrantMesh(UICorner.All, _roundedRadius, 6, false,
				new UILathePt { pt = V.XY(0, -0.5f), color = Color32.White, normal = V.XY(0, 1), connectNext = true },
				new UILathePt { pt = V.XY(1, -0.5f), color = Color32.White, normal = V.XY(0, 1), connectNext = false });

			return _roundedMesh;
		}

		/// <summary>If you know you won't need RoundedSprites anymore, you can
		/// free up the Mesh. It'll just re-allocate again if re-used, so no
		/// worries about calling this whenever.</summary>
		public static void ClearMesh()
		{
			_roundedMesh   = null;
			_roundedRadius = 0;
		}
	}
}