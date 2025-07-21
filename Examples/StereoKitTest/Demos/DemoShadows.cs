using StereoKit;
using System;
using System.Runtime.InteropServices;

class DemoShadows : ITest
{
	[StructLayout(LayoutKind.Sequential)]
	struct ShadowBuffer
	{
		public Matrix transform;
		public float texSize;
		public float bias;
		public float biasSlope;
		public float depthDistance;
		public Vec3 lightDirection;
		public float farScale;
		public Vec3 lightColor;
		public float biasFar;
	}

	Tex[] shadowMap    = new Tex[2];
	Tex[] shadowMapFar = new Tex[2];
	Model model;
	MaterialBuffer<ShadowBuffer> shadowBuffer;
	ShadowBuffer last;
	Mesh cylinder;
	Material shadowMat;

	SphericalHarmonics oldLighting;
	Tex oldTex;

	public void Initialize()
	{ 
		shadowBuffer = new MaterialBuffer<ShadowBuffer>(12);
		const int resolution = 512;
		for (int i = 0; i < shadowMap.Length; i++) {
			shadowMap[i] = Tex.RenderTarget(resolution, resolution, 1, TexFormat.None, TexFormat.Depth16);
			shadowMap[i].SampleMode  = TexSample.Linear;
			shadowMap[i].SampleComp  = TexSampleComp.Less;
			shadowMap[i].AddressMode = TexAddress.Clamp;
			shadowMap[i].Id          = "app/shadow/zbuffer/"+i;

			shadowMapFar[i] = Tex.RenderTarget(resolution, resolution, 1, TexFormat.None, TexFormat.Depth16);
			shadowMapFar[i].SampleMode  = TexSample.Linear;
			shadowMapFar[i].SampleComp  = TexSampleComp.Less;
			shadowMapFar[i].AddressMode = TexAddress.Clamp;
			shadowMapFar[i].Id          = "app/shadow/zbuffer/far_"+i;
		}

		model = Model.FromFile("Townscaper.glb", Shader.FromFile("Shaders/basic_shadow.hlsl"));
		cylinder = Mesh.GenerateCylinder(0.1f, 10, Vec3.Up);
		shadowMat = new Material("Shaders/basic_shadow.hlsl");

		oldLighting = Renderer.SkyLight;
		oldTex      = Renderer.SkyTex;

		SphericalHarmonics ambient = new SphericalHarmonics();
		ambient.Add( Vec3.UnitY, Color.HSV(0.55f, 0.3f,  0.4f).ToLinear());
		ambient.Add(-Vec3.UnitY, Color.HSV(0.0f,  0.05f, 0.2f).ToLinear());
		SphericalHarmonics ambientSky = ambient;
		ambientSky.Brightness(2.5f);

		Renderer.SkyLight = ambient;
		Renderer.SkyTex   = Tex.GenCubemap(ambientSky);
	}

	public void Shutdown()
	{
		Renderer.SkyLight = oldLighting;
		Renderer.SkyTex   = oldTex;
	}

	Pose sunPose = new Pose(V.XYZ(0.2f,0.6f,-1.001f)*20);
	float angle = 1;
	float bias      = 4.0f / ushort.MaxValue;
	float biasSlope = 40.0f / ushort.MaxValue;
	public void Step()
	{
		if (Input.Key(Key.K).IsActive()) angle -= 0.01f;
		if (Input.Key(Key.L).IsActive()) angle += 0.01f;


		if (Input.Key(Key.H).IsJustActive()) { bias += 1.0f / ushort.MaxValue; Log.Info($"Bias: {bias*ushort.MaxValue}"); }
		if (Input.Key(Key.N).IsJustActive()) { bias -= 1.0f / ushort.MaxValue; Log.Info($"Bias: {bias*ushort.MaxValue}"); }

		if (Input.Key(Key.J).IsJustActive()) { biasSlope += 1.0f / ushort.MaxValue; Log.Info($"Bias Slope: {biasSlope*ushort.MaxValue}"); }
		if (Input.Key(Key.M).IsJustActive()) { biasSlope -= 1.0f / ushort.MaxValue; Log.Info($"Bias Slope: {biasSlope*ushort.MaxValue}"); }

		UI.Handle("sun", ref sunPose, new Bounds(Vec3.One*0.1f));
		//sunPose.position = Vec3.AngleXZ(Time.Totalf*90, 1);
		//Vec3 sunDir    = new Vec3(0, -1, 0);
		//Vec3 shadowPos = (sunDir.Normalized * -4); // Input.Head.position - (sunDir.Normalized * 4);
		//sunPose.position = V.XYZ((float)Math.Cos(Time.Total*0.25), 1, (float)Math.Sin(Time.Total*0.25)) * 10;
		//sunPose.position = V.XYZ((float)Math.Cos(Time.Total*0.25), (float)Math.Sin(Time.Total*0.25), 0) * 10;
		//sunPose.position = V.XYZ(0, (float)Math.Sin(angle), (float)Math.Cos(angle)) * 10;
		//Vec3 shadowPos = sunPose.position;
		//Vec3 sunLookAt = new Vec3(0,-1.5f,0);
		//Vec3 sunDir    = (sunLookAt-sunPose.position).Normalized;


		float size    = 2;
		float farSize = size * 10;

		Pose head = Input.Head;
		Vec3 forward    = head.Forward.X0Z.Normalized;
		Vec3 forwardPos = head.position.X0Z + forward * 0.5f * size;

		// Stabilize the shadow map so we don't get sparkles when moving the camera
		Vec3 sunDir = -V.XYZ(0, (float)Math.Sin(angle), (float)Math.Cos(angle));
		sunPose.position    = sunDir * -10;
		sunPose.orientation = Quat.LookAt(Vec3.Zero, sunDir, Vec3.Up);
		Vec3 localP      = forwardPos - sunPose.position;
		Vec2 planeCoords = new Vec2(
			Vec3.Dot(localP, sunPose.orientation * Vec3.UnitX),
			Vec3.Dot(localP, sunPose.orientation * Vec3.UnitY) );
		float texelSize = farSize / shadowMapFar[0].Width;
		Vec2  texCoords = planeCoords / texelSize;
		texCoords = new Vec2((float)Math.Round(texCoords.x), (float)Math.Round(texCoords.y));
		Vec2 quantizedPlaneCoords = texCoords * texelSize;
		Vec3 quantizedPos = sunPose.position +
			(sunPose.orientation * Vec3.UnitX) * quantizedPlaneCoords.x +
			(sunPose.orientation * Vec3.UnitY) * quantizedPlaneCoords.y;

		sunPose.position = quantizedPos;
		//sunPose.position    = forwardPos - sunDir * 10;


		Matrix view = Matrix.TR(sunPose.position, sunPose.orientation);
		//Matrix view = Matrix.TR          (shadowPos, Quat.LookDir(sunDir));
		Matrix proj    = Matrix.Orthographic(size,    size,    0.01f, 20);
		Matrix projFar = Matrix.Orthographic(farSize, farSize, 0.01f, 20);

		int renderTo   = (int)( Time.Frame                                  % (ulong)shadowMap.Length);
		int renderFrom = (int)((Time.Frame + ((ulong)shadowMap.Length - 1)) % (ulong)shadowMap.Length);

		Color lightColor = Color.HSV(0.16f, 0.1f, 1.0f).ToLinear();
		ShadowBuffer curr = new ShadowBuffer
		{
			transform = (view.Inverse * proj).Transposed,
			texSize   = 1.0f / shadowMap[renderTo].Width,
			bias      = bias,
			biasSlope = biasSlope,
			biasFar   = biasSlope,
			depthDistance = 20.0f - 0.01f,
			lightDirection = -sunDir,
			farScale = farSize/size,
			lightColor = V.XYZ(lightColor.r, lightColor.g, lightColor.b),
		};
		shadowBuffer.Set(last);
		last = curr;

		Renderer.RenderTo(shadowMap   [renderTo], view, proj,    RenderLayer.All &~ RenderLayer.Vfx);
		Renderer.RenderTo(shadowMapFar[renderTo], view, projFar, RenderLayer.All &~ RenderLayer.Vfx);
		Renderer.SetGlobalTexture(12, shadowMap   [renderFrom]);
		Renderer.SetGlobalTexture(13, shadowMapFar[renderFrom]);

		//Mesh.Quad  .Draw(shadowMat, Matrix.TRS(new Vec3(0,-2.5f,0),Quat.LookAt(Vec3.Zero, Vec3.UnitY, Vec3.UnitZ),4));
		//Mesh.Sphere.Draw(shadowMat, Matrix.TS (new Vec3(0,-2,-0.5f), 0.5f));
		model.Draw(Matrix.S(0.1f));
		cylinder.Draw(shadowMat, Matrix.T(-0.2f,0,0));

		Mesh.Sphere.Draw(Material.Unlit, Matrix.TS(sunPose.position, 0.1f), Color.White, RenderLayer.Vfx);
		Lines.Add(sunPose.position + sunPose.Right*size*0.5f, sunPose.position - sunPose.Right*size*0.5f, new Color32(255,200,200,255), 0.01f);
		Lines.Add(sunPose.position + sunPose.Up   *size*0.5f, sunPose.position - sunPose.Up   *size*0.5f, new Color32(200,255,200,255), 0.01f);
		//Lines.Add(sunPose.position, sunPose.position + sunPose.Forward * size * (20.0f - 0.01f), new Color32(200, 255, 200, 255), 0.01f);
	}
}
