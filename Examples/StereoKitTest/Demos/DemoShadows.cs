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

	const float ShadowMapSize       = 2;
	const int   ShadowMapResolution = 1024;
	const float ShadowMapNearClip   = 0.01f;
	const float ShadowMapFarClip    = 20.0f;

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

		Material shadowMat = new Material("Shaders/basic_shadow.hlsl");
		shadowMat.DepthTest = DepthTest.LessOrEq;
		Material floorMat = shadowMat.Copy();
		floorMat[MatParamName.DiffuseTex] = Tex.FromFile("floor.png");
		floorMat[MatParamName.TexTransform] = new Vec4(0, 0, 2, 2);
		model = GenerateModel(floorMat, shadowMat);

		oldLighting = Renderer.SkyLight;
		oldTex      = Renderer.SkyTex;

		SphericalHarmonics ambient = new SphericalHarmonics();
		ambient.Add( Vec3.UnitY, Color.HSV(0.55f, 0.3f,  0.6f).ToLinear());
		ambient.Add(-Vec3.UnitY, Color.HSV(0.0f,  0.01f, 0.2f).ToLinear());
		SphericalHarmonics ambientSky = ambient;

		Renderer.SkyLight = ambient;
		Renderer.SkyTex   = Tex.GenCubemap(ambientSky);
		Renderer.SetGlobalBuffer(12, shadowBuffer);
	}

	public void Shutdown()
	{
		Renderer.SkyLight = oldLighting;
		Renderer.SkyTex   = oldTex;
		Renderer.SetGlobalBuffer(12, null);
	}
	public void Step()
	{
		// Make a direction for the light
		const float angleX = Units.deg2rad * 45;
		const float angleY = Units.deg2rad * 45;
		float cosX = (float)Math.Cos(angleX);
		float sinX = (float)Math.Sin(angleX);
		float cosY = (float)Math.Cos(angleY);
		float sinY = (float)Math.Sin(angleY);
		SetupShadowMap(-V.XYZ(cosX * sinY, sinX, cosX * cosY));

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
		Renderer.RenderTo(shadowMap, view, proj, RenderLayer.All &~ RenderLayer.Vfx);
		Renderer.SetGlobalTexture(12, shadowMap);
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