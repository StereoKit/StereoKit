// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;

class DemoMaterial : ITest
{
	string title = "Materials";
	string description = "";

	Mesh meshSphere;

	Material matDefault;
	Material matWireframe;
	Material matCull;
	Material matTextured;
	Material matMSAABlend;
	Material matAlphaBlend;
	Material matAlphaAdd;
	Material matUnlit;
	Material matParameters;
	Material matPBR;
	Material matUIBox;
	Material matUI;

	Tex                oldSkyTex;
	SphericalHarmonics oldSkyLight;

	public void Initialize()
	{
		oldSkyTex         = Renderer.SkyTex;
		oldSkyLight       = Renderer.SkyLight;

		/// :CodeSample: Renderer.SkyTex Renderer.SkyLight Tex.FromCubemapEquirectangular
		/// ### Setting lighting to an equirect cubemap
		/// Changing the environment's lighting based on an image is a really
		/// great way to instantly get a particular feel to your scene! A neat
		/// place to find compatible equirectangular images for this is
		/// [Poly Haven](https://polyhaven.com/hdris)
		Renderer.SkyTex   = Tex.FromCubemapEquirectangular("old_depot.hdr", out SphericalHarmonics lighting);
		Renderer.SkyLight = lighting;
		/// And here's what it looks like applied to the default Material!
		/// ![Default Material example]({{site.screen_url}}/MaterialDefault.jpg)
		/// :End:

		meshSphere = Mesh.GenerateSphere(1,8);

		/// :CodeSample: Default.Material Material.Default
		/// If you want to modify the default material, it's recommended to
		/// copy it first!
		matDefault = Material.Default.Copy();
		/// And here's what it looks like:
		/// ![Default Material example]({{site.screen_url}}/MaterialDefault.jpg)
		/// :End:

		/// :CodeSample: Default.MaterialUI Material.UI
		/// This Material is basically the same as Default.Material, except it
		/// also adds some glow to the surface near the user's fingers. It
		/// works best on flat surfaces, and in StereoKit's design language,
		/// can be used to indicate that something is interactive.
		matUI = Material.UI.Copy();
		/// And here's what it looks like:
		/// ![UI Material example]({{site.screen_url}}/MaterialUI.jpg)
		/// :End:

		/// :CodeSample: Material.Wireframe
		/// Here's creating a simple wireframe material!
		matWireframe = Material.Default.Copy();
		matWireframe.Wireframe = true;
		/// Which looks like this:
		/// ![Wireframe material example]({{site.screen_url}}/MaterialWireframe.jpg)
		/// :End:

		/// :CodeSample: Material.FaceCull Cull.Front
		/// Here's setting FaceCull to Front, which is the opposite of the
		/// default behavior. On a sphere, this is a little hard to see, but
		/// you might notice here that the lighting is for the back side of
		/// the sphere!
		matCull = Material.Default.Copy();
		matCull.FaceCull = Cull.Front;
		/// ![FaceCull material example]({{site.screen_url}}/MaterialCull.jpg)
		/// :End:

		/// :CodeSample: Material.Transparency Transparency.Add
		/// ### Additive Transparency
		/// Here's an example material with additive transparency.
		/// Transparent materials typically don't write to the depth buffer,
		/// but this may vary from case to case. Note that the material's
		/// alpha does not play any role in additive transparency! Instead,
		/// you could make the material's tint darker.
		matAlphaAdd = Material.Default.Copy();
		matAlphaAdd.Transparency = Transparency.Add;
		matAlphaAdd.DepthWrite   = false;
		/// ![Additive transparency example]({{site.screen_url}}/MaterialAlphaAdd.jpg)
		/// :End:

		/// :CodeSample: Material.Transparency Transparency.Blend
		/// ### Alpha Blending
		/// Here's an example material with an alpha blend transparency.
		/// Transparent materials typically don't write to the depth buffer,
		/// but this may vary from case to case. Here we're setting the alpha
		/// through the material's Tint value, but the diffuse texture's
		/// alpha and the instance render color's alpha may also play a part
		/// in the final alpha value.
		matAlphaBlend = Material.Default.Copy();
		matAlphaBlend.Transparency = Transparency.Blend;
		matAlphaBlend.DepthWrite   = false;
		matAlphaBlend[MatParamName.ColorTint] = new Color(1, 1, 1, 0.75f);
		/// ![Alpha blend example]({{site.screen_url}}/MaterialAlphaBlend.jpg)
		/// :End:

		/// :CodeSample: Material.Transparency Transparency.MSAA
		/// ### MSAA (Alpha to Coverage)
		/// Here's an example material with a transparency mode that utilizes
		/// MSAA samples for blending. Also known as Alpha To Coverage, this
		/// takes advantage of the fact that MSAA can generate multiple
		/// fragments per-pixel while utilizing the zbuffer, and then blend
		/// them together before presenting the image. This means you can dodge
		/// a couple of z-sorting artifacts, but with a limited/quantized
		/// number of transparency "values" equivalent to the number of MSAA
		/// samples.
		matMSAABlend = Material.Default.Copy();
		matMSAABlend.Transparency = Transparency.MSAA;
		matMSAABlend[MatParamName.ColorTint] = new Color(1, 1, 1, 0.75f);
		/// ![MSAA transparency example]({{site.screen_url}}/MaterialMSAABlend.jpg)
		/// :End:

		matTextured = Material.Default.Copy();
		matTextured[MatParamName.DiffuseTex] = Tex.FromFile("floor.png");

		/// :CodeSample: Default.MaterialUnlit Material.Unlit
		matUnlit = Material.Unlit.Copy();
		matUnlit[MatParamName.DiffuseTex] = Tex.FromFile("floor.png");
		/// ![Unlit material example]({{site.screen_url}}/MaterialUnlit.jpg)
		/// :End:

		/// :CodeSample: Default.MaterialPBR Material.PBR
		/// Occlusion (R), Roughness (G), and Metal (B) are stored
		/// respectively in the R, G and B channels of their texture.
		/// Occlusion can be separated out into a different texture as per
		/// the GLTF spec, so you do need to assign it separately from the
		/// Metal texture.
		matPBR = Material.PBR.Copy();
		matPBR[MatParamName.DiffuseTex  ] = Tex.FromFile("metal_plate_diff.jpg");
		matPBR[MatParamName.MetalTex    ] = Tex.FromFile("metal_plate_metal.jpg", false);
		matPBR[MatParamName.OcclusionTex] = Tex.FromFile("metal_plate_metal.jpg", false);
		/// ![PBR material example]({{site.screen_url}}/MaterialPBR.jpg)
		/// :End:

		/// :CodeSample: Default.MaterialUIBox Material.UIBox
		/// The UI Box material has 3 parameters to control how the box wires
		/// are rendered. The initial size in meters is 'border_size', and
		/// can grow by 'border_size_grow' meters based on distance to the
		/// user's hand. That distance can be configured via the
		/// 'border_affect_radius' property of the shader, which is also in
		/// meters.
		matUIBox = Material.UIBox.Copy();
		matUIBox["border_size"]          = 0.005f;
		matUIBox["border_size_grow"]     = 0.01f;
		matUIBox["border_affect_radius"] = 0.2f;
		/// ![UI box material example]({{site.screen_url}}/MaterialUIBox.jpg)
		/// :End:

		matParameters = Material.Default.Copy();
		matParameters[MatParamName.DiffuseTex  ] = Tex.FromFile("floor.png");
		matParameters[MatParamName.ColorTint   ] = Color.HSV(0.6f, 0.7f, 1f);
		matParameters[MatParamName.TexTransform] = new Vec4(0,0,2,2);
	}

	int showCount;
	int showGrid = 3;
	void ShowMaterial(Mesh mesh, Material material, string screenshotName, Vec3? from = null) 
	{
		float x = ((showCount % showGrid)-showGrid/2) * 0.16f;
		float y = ((showCount / showGrid)-showGrid/2) * 0.16f;
		showCount++;

		Vec3 at = Demo.contentPose.Transform(new Vec3(x, y-0.08f, 0));
		mesh.Draw(material, Matrix.TS(at, 0.1f));
		Tests.Screenshot(screenshotName, 0, 400, 400, 45, at + (from ?? new Vec3(0, 0, -0.16f)), at);
	}

	public void Step()
	{
		showCount=0;
		ShowMaterial(meshSphere, matDefault,    "MaterialDefault.jpg");
		ShowMaterial(meshSphere, matWireframe,  "MaterialWireframe.jpg");
		ShowMaterial(meshSphere, matCull,       "MaterialCull.jpg");
		ShowMaterial(meshSphere, matTextured,   "MaterialTextured.jpg");
		ShowMaterial(meshSphere, matAlphaAdd,   "MaterialAlphaAdd.jpg");
		ShowMaterial(meshSphere, matAlphaBlend, "MaterialAlphaBlend.jpg");
		ShowMaterial(meshSphere, matMSAABlend,  "MaterialMSAABlend.jpg");
		ShowMaterial(meshSphere, matUnlit,      "MaterialUnlit.jpg");
		ShowMaterial(meshSphere, matPBR,        "MaterialPBR.jpg");
		ShowMaterial(meshSphere, matParameters, "MaterialParameters.jpg");
		ShowMaterial(Mesh.Cube,  matUIBox,      "MaterialUIBox.jpg", new Vec3( 0.14f, 0.14f, -0.16f));
		ShowMaterial(Mesh.Cube,  matUI,         "MaterialUI.jpg",    new Vec3(-0.14f, 0.14f, -0.16f));

		Demo.ShowSummary(title, description, new Bounds(.5f, .7f, 0.2f));
	}

	public void Shutdown()
	{
		Renderer.SkyTex   = oldSkyTex;
		Renderer.SkyLight = oldSkyLight;
	}
}