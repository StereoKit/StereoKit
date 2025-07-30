using StereoKit;
using StereoKit.Framework;
using System;
using System.Runtime.InteropServices;

class DemoShadows : ITest
{
	DynamicShadows shadows;

	Model    model;
	Mesh     cylinder;
	Material shadowMat;

	SphericalHarmonics oldLighting;
	Tex                oldTex;

	public void Initialize()
	{
		shadows = SK.AddStepper(new DynamicShadows(1, Color.HSV(0.16f, 0.1f, 1.0f), 512));

		model = Model.FromFile("Townscaper.glb", Shader.FromFile("Shaders/basic_shadow.hlsl"));
		//model = Model.FromFile(@"C:\Data\GLTF\marsh.glb", Shader.FromFile("Shaders/basic_shadow.hlsl"));
		model.PlayAnim(model.Anims[0], AnimMode.Loop);
		cylinder = Mesh.GenerateCylinder(0.1f, 10, Vec3.Up);
		shadowMat = new Material("Shaders/basic_shadow.hlsl");

		oldLighting = Renderer.SkyLight;
		oldTex      = Renderer.SkyTex;

		SphericalHarmonics ambient = new SphericalHarmonics();
		ambient.Add( Vec3.UnitY, Color.HSV(0.55f, 0.3f,  0.4f).ToLinear());
		ambient.Add(-Vec3.UnitY, Color.HSV(0.0f,  0.01f, 0.15f).ToLinear());
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
	Vec3 sunDir = -Vec3.UnitY;
	float angleX = 1;
	float angleY = 1;
	float bias      = 50.0f  / ushort.MaxValue;
	float biasSlope = 800.0f / ushort.MaxValue;
	public void Step()
	{
		if (Input.Key(Key.K).IsActive()) angleX  -= 0.01f;
		if (Input.Key(Key.L).IsActive()) angleX  += 0.01f;

		if (Input.Key(Key.I).IsActive()) angleY -= 0.01f;
		if (Input.Key(Key.O).IsActive()) angleY += 0.01f;

		Controller c = Input.Controller(Handed.Right);
		//if (c.grip > 0.5f)
		//{
		//	sunDir = c.stick.X0Y;
		//	sunDir.y = -1;
		//	sunDir = sunDir.Normalized;
		//}
		//else
		{
			float cosX = (float)Math.Cos(angleX);
			float sinX = (float)Math.Sin(angleX);
			float cosY = (float)Math.Cos(angleY);
			float sinY = (float)Math.Sin(angleY);
			sunDir = -V.XYZ(
				cosX * sinY,
				sinX,
				cosX * cosY);
		}


		if (Input.Key(Key.H).IsJustActive()) { bias += 10.0f / ushort.MaxValue; Log.Info($"Bias: {bias * ushort.MaxValue}"); }
		if (Input.Key(Key.N).IsJustActive()) { bias -= 10.0f / ushort.MaxValue; Log.Info($"Bias: {bias*ushort.MaxValue}"); }

		if (Input.Key(Key.J).IsJustActive()) { biasSlope += 10.0f / ushort.MaxValue; Log.Info($"Bias Slope: {biasSlope*ushort.MaxValue}"); }
		if (Input.Key(Key.M).IsJustActive()) { biasSlope -= 10.0f / ushort.MaxValue; Log.Info($"Bias Slope: {biasSlope*ushort.MaxValue}"); }

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

		shadows.LightDirection = sunDir;

		//Mesh.Quad  .Draw(shadowMat, Matrix.TRS(new Vec3(0,-2.5f,0),Quat.LookAt(Vec3.Zero, Vec3.UnitY, Vec3.UnitZ),4));
		//Mesh.Sphere.Draw(shadowMat, Matrix.TS (new Vec3(0,-2,-0.5f), 0.5f));
		model.Draw(Matrix.S(0.1f));
		cylinder.Draw(shadowMat, Matrix.T(-0.2f,0,0));
		//cylinder.Draw(Default.MaterialHand, Matrix.T(-0.2f,0,0));

		Mesh.Sphere.Draw(Material.Unlit, Input.Hand(Handed.Right).palm.ToMatrix(0.01f), Color.White, RenderLayer.Vfx);
	}
}

public class DynamicShadows : IStepper
{
	[StructLayout(LayoutKind.Sequential)]
	struct ShadowBuffer
	{
		public Matrix transform;
		public float texSize;
		public float bias;
		public float biasSlope;
		public float depthDistance;
		public Vec3  lightDirection;
		public float farScale;
		public Vec3  lightColor;
		public float biasFar;
	}

	Tex[] shadowMap = new Tex[2];
	MaterialBuffer<ShadowBuffer> shadowBuffer;
	ShadowBuffer last;

	private Color _lightColor;

	public int   Resolution { get; private set; }
	public Color LightColor { get => _lightColor.ToGamma(); set => _lightColor = value.ToLinear(); }
	public float Size;

	public float Bias      = 50.0f  / ushort.MaxValue;
	public float BiasSlope = 800.0f / ushort.MaxValue;

	public float NearClip          = 0.01f;
	public float FarClip           = 20;
	public float CascadeMultiplier = 8;
	public Vec3  LightDirection;

	public bool Enabled => true;

	public DynamicShadows(float sizeMeters, Color colorGamma, int resolution = 512)
	{
		Size       = sizeMeters;
		Resolution = resolution;
		LightColor = colorGamma;
	}

	public bool Initialize()
	{
		shadowBuffer = new MaterialBuffer<ShadowBuffer>(12);
		for (int i = 0; i < shadowMap.Length; i++) {
			shadowMap[i] = new Tex(TexType.Depthtarget, TexFormat.Depth16);
			shadowMap[i].SetSize(Resolution, Resolution, 2);
			shadowMap[i].SampleMode  = TexSample.Linear;
			shadowMap[i].SampleComp  = TexSampleComp.Less;
			shadowMap[i].AddressMode = TexAddress.Clamp;
			shadowMap[i].Id          = "app/shadow/zbuffer/"+i;
		}
		return true;
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		Pose head       = Input.Head;
		Vec3 forward    = head.Forward.X0Z.Normalized;
		Vec3 forwardPos = head.position.X0Z + forward * 0.5f * Size;
		Vec3 lightDir   = LightDirection.Normalized;

		// Stabilize the shadow map so we don't get sparkles when moving the camera
		Pose sunPose;
		sunPose.position    = lightDir * -10;
		sunPose.orientation = Quat.LookAt(Vec3.Zero, lightDir, Vec3.Up);
		Vec3 localP      = forwardPos - sunPose.position;
		Vec2 planeCoords = new Vec2(
			Vec3.Dot(localP, sunPose.orientation * Vec3.UnitX),
			Vec3.Dot(localP, sunPose.orientation * Vec3.UnitY) );
		float texelSize = (Size * CascadeMultiplier) / Resolution;
		Vec2  texCoords = planeCoords / texelSize;
		texCoords = new Vec2((float)Math.Round(texCoords.x), (float)Math.Round(texCoords.y));
		Vec2 quantizedPlaneCoords = texCoords * texelSize;
		Vec3 quantizedPos = sunPose.position +
			(sunPose.orientation * Vec3.UnitX) * quantizedPlaneCoords.x +
			(sunPose.orientation * Vec3.UnitY) * quantizedPlaneCoords.y;

		sunPose.position = quantizedPos;

		Matrix view    = Matrix.TR(sunPose.position, sunPose.orientation);
		Matrix proj    = Matrix.Orthographic(Size,                   Size,                   NearClip, FarClip);
		Matrix projFar = Matrix.Orthographic(Size*CascadeMultiplier, Size*CascadeMultiplier, NearClip, FarClip);

		int renderTo   = (int)( Time.Frame                                  % (ulong)shadowMap.Length);
		int renderFrom = (int)((Time.Frame + ((ulong)shadowMap.Length - 1)) % (ulong)shadowMap.Length);

		ShadowBuffer curr = new ShadowBuffer
		{
			transform      = (view.Inverse * proj).Transposed,
			texSize        = 1.0f / shadowMap[renderTo].Width,
			bias           = Bias,
			biasSlope      = BiasSlope,
			biasFar        = BiasSlope,
			depthDistance  = FarClip - NearClip,
			lightDirection = -lightDir,
			farScale       = CascadeMultiplier,
			lightColor     = V.XYZ(_lightColor.r, _lightColor.g, _lightColor.b),
		};
		shadowBuffer.Set(last);
		last = curr;

		Renderer.RenderTo(shadowMap[renderTo], 0, view, proj,    RenderLayer.All &~ RenderLayer.Vfx);
		Renderer.RenderTo(shadowMap[renderTo], 1, view, projFar, RenderLayer.All &~ RenderLayer.Vfx);
		Renderer.SetGlobalTexture(12, shadowMap[renderFrom]);
	}

	public void DebugDraw()
	{
	}
}