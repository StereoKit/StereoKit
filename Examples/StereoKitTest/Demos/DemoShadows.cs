using StereoKit;
using System;
using System.Runtime.InteropServices;

class DemoShadows : ITest
{
	string title       = "Dynamic ShadowMaps";
	string description = "Using StereoKit's Renderer.RenderTo, SetGlobalTexture, MaterialBuffers, and a bit of ingenuity, you can also add shadow mapping to your app!\n\nThis is a very basic single cascade implementation of shadow mapping to illustrate the idea.";

	[StructLayout(LayoutKind.Sequential)]
	struct ShadowBuffer
	{
		public Matrix shadowMapTransform;
		public Vec3   lightDirection;
		public float  shadowMapBias;
		public Vec3   lightColor;
		public float  shadowMapPixelSize;
	}

	Tex                          shadowMap;
	MaterialBuffer<ShadowBuffer> shadowBuffer;
	Vec3                         lightDir = new Vec3(1,1,0).Normalized;

	const float ShadowMapSize       = 2;
	const int   ShadowMapResolution = 1024;
	const float ShadowMapNearClip   = 0.01f;
	const float ShadowMapFarClip    = 20.0f;
	const int   ShadowMapVariant    = 1;

	Model model;
	Pose  modelPose = Matrix.T(0,-0.2f,0) * Demo.contentPose.Pose;

	SphericalHarmonics oldLighting;
	Tex                oldTex;

	public void Initialize()
	{
		shadowBuffer = new MaterialBuffer<ShadowBuffer>();
		shadowMap = new Tex(TexType.Depthtarget, TexFormat.Depth16);
		shadowMap.SetSize(ShadowMapResolution, ShadowMapResolution);
		shadowMap.SampleMode  = TexSample.Linear;
		shadowMap.SampleComp  = TexSampleComp.LessOrEq;
		shadowMap.AddressMode = TexAddress.Clamp;
		
		Material casterMat = new Material("Shaders/basic_shadow_caster.hlsl");
		casterMat.DepthClamp = true;
		casterMat.DepthTest = DepthTest.LessOrEq;
		// This can help with shadow acne if biasing isn't working out, but can introduce peter-panning.
		//casterMat.FaceCull  = Cull.Front;

		Material shadowMat = new Material("Shaders/basic_shadow.hlsl");
		shadowMat.SetVariant(ShadowMapVariant, casterMat);

		Material floorMat = shadowMat.Copy();
		floorMat[MatParamName.DiffuseTex] = Tex.FromFile("floor.png");
		floorMat[MatParamName.TexTransform] = new Vec4(0, 0, 2, 2);
		model = GenerateModel(floorMat, shadowMat);

		oldLighting = Renderer.SkyLight;
		oldTex      = Renderer.SkyTex;

		Renderer.SkyTex = Tex.FromCubemap(@"old_depot.hdr");
		Renderer.SkyTex.OnLoaded += t => { Renderer.SkyLight = t.CubemapLighting; lightDir = t.CubemapLighting.DominantLightDirection; };

		Renderer.SetGlobalBuffer(13, shadowBuffer);
	}

	public void Shutdown()
	{
		Renderer.SkyLight = oldLighting;
		Renderer.SkyTex   = oldTex;
		Renderer.SetGlobalBuffer(13, null);
	}
	public void Step()
	{
		SetupShadowMap(lightDir);

		UI.Handle("Model", ref modelPose, model.Bounds);
		model.Draw(modelPose.ToMatrix());

		Demo.ShowSummary(title, description,
			new Bounds(V.XY0(0, -0.05f), V.XYZ(.6f, .4f, 0.6f)));
	}

	void SetupShadowMap(Vec3 lightDir)
	{
		// Position the center of the shadow map in front of the user.
		Pose head = Input.Head;
		Vec3 forwardPos       = head.position.X0Z + head.Forward.X0Z.Normalized * 0.5f * ShadowMapSize;
		Quat lightOrientation = Quat.LookAt(Vec3.Zero, lightDir, Vec3.Up);
		Vec3 lightPos         = QuantizeLightPos( forwardPos + lightDir * -10, lightOrientation, ShadowMapSize/ShadowMapResolution );

		// Create rendering matrices for the shadow map
		Matrix view = Matrix.TR(lightPos, lightOrientation);
		Matrix proj = Matrix.Orthographic(ShadowMapSize, ShadowMapSize, ShadowMapNearClip, ShadowMapFarClip);

		// Send information about the shadow map parameters to the renderer,
		// these are used by the basic_shadow.hlsl shader.
		shadowBuffer.Set(new ShadowBuffer {
			shadowMapTransform = (view.Inverse * proj).Transposed,
			shadowMapBias      = 2 * MathF.Max(((ShadowMapFarClip - ShadowMapNearClip) / ushort.MaxValue), ShadowMapSize / ShadowMapResolution),
			lightDirection     = -lightDir,
			lightColor         =  V.XYZ(1,1,1),
			shadowMapPixelSize = 1.0f / ShadowMapResolution
		});

		// Render the shadow map, and bind it globally so it can be used from
		// any shader.
		Renderer.SetGlobalTexture(13, null); // Can't draw to it if it's bound
		Renderer.RenderTo(shadowMap, view, proj, RenderLayer.All &~ RenderLayer.Vfx, ShadowMapVariant);
		Renderer.SetGlobalTexture(13, shadowMap);
	}

	static Model GenerateModel(Material floorMat, Material material)
	{
		
		const float width  = 0.5f;
		const float height = 0.5f;
		Vec3  sizeMin = new Vec3(0.04f, 0.04f, 0.04f);
		Vec3  sizeMax = new Vec3(.06f, .2f, .06f);
		float genWidth  = width - sizeMax.x;
		float genHeight = height - sizeMax.z;
		Model model = new Model();
		model.AddNode("Floor", Matrix.S(width, 0.02f, height), Mesh.Cube, floorMat);

		Random r = new Random(1);
		for (int i = 0; i < 20; i++)
		{
			float x = (r.NextSingle() - 0.5f) * genWidth;
			float y = (r.NextSingle() - 0.5f) * genHeight;
			Vec3 size = new Vec3(
				sizeMin.x + r.NextSingle() * (sizeMax.x - sizeMin.x),
				sizeMin.y + r.NextSingle() * (sizeMax.y - sizeMin.y),
				sizeMin.z + r.NextSingle() * (sizeMax.z - sizeMin.z));
			model.AddNode("Cube"+i, Matrix.TS(x, 0.01f + size.y/2, y, size), Mesh.Cube, material);
		}

		return model;
	}

	static Vec3 QuantizeLightPos(Vec3 pos, Quat lightOrientation, float texelSize)
	{
		Vec3  right = lightOrientation * Vec3.UnitX;
		Vec3  up    = lightOrientation * Vec3.UnitY;
		Plane p     = new Plane(lightOrientation * Vec3.UnitZ, 0);

		Vec3 localP   = p.Closest(pos);
		Vec3 localOff = pos - localP;
		Vec2 planeCoords = new Vec2(Vec3.Dot(localP, right), Vec3.Dot(localP, up));
		Vec2 texCoords   = planeCoords / texelSize;
		texCoords = new Vec2((float)Math.Round(texCoords.x), (float)Math.Round(texCoords.y));
		Vec2 quantizedPlaneCoords = texCoords * texelSize;
		return localOff +
			right * quantizedPlaneCoords.x +
			up    * quantizedPlaneCoords.y;
	}
}