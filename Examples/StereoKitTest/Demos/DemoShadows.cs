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
		public float depthDistance;
		float dummy;
		public Vec3 lightDirection;
		float dummy2;
	}

	Tex[]    shadowMap  = new Tex[2];
	Material shadowMat  = new Material("Shaders/basic_shadow.hlsl");
	Model model;
	MaterialBuffer<ShadowBuffer> shadowBuffer;
	ShadowBuffer last;

	public void Initialize()
	{ 
		shadowBuffer = new MaterialBuffer<ShadowBuffer>(12);
		for (int i = 0; i < shadowMap.Length; i++) {
			shadowMap[i] = Tex.RenderTarget(2048, 2048, 1, TexFormat.None, TexFormat.Depth16);
			shadowMap[i].SampleMode = TexSample.Point;
			shadowMap[i].Id = "app/shadow/zbuffer/"+i;
		}

		model = Model.FromFile("Townscaper.glb", Shader.FromFile("Shaders/basic_shadow.hlsl"));
	}

	public void Shutdown()
	{
	}

	Pose sunPose = new Pose(V.XYZ(0.2f,0.6f,-1.001f)*20);
	public void Step()
	{
		UI.Handle("sun", ref sunPose, new Bounds(Vec3.One*0.1f));
		//sunPose.position = Vec3.AngleXZ(Time.Totalf*90, 1);
		//Vec3 sunDir    = new Vec3(0, -1, 0);
		//Vec3 shadowPos = (sunDir.Normalized * -4); // Input.Head.position - (sunDir.Normalized * 4);
		sunPose.position = V.XYZ((float)Math.Cos(Time.Total*0.25), 1, (float)Math.Sin(Time.Total*0.25)) * 10;
		Vec3 shadowPos = sunPose.position;
		Vec3 sunLookAt = new Vec3(0,-1.5f,0);
		Vec3 sunDir    = (sunLookAt-sunPose.position).Normalized;

		float size = 5;
		Matrix view = Matrix.TR          (shadowPos, Quat.LookDir(sunDir));
		Matrix proj = Matrix.Orthographic(size, size, 0.01f, 20);

		int renderTo   = (int)( Time.Frame                                  % (ulong)shadowMap.Length);
		int renderFrom = (int)((Time.Frame + ((ulong)shadowMap.Length - 1)) % (ulong)shadowMap.Length);

		ShadowBuffer curr = new ShadowBuffer
		{
			transform = (view.Inverse * proj).Transposed,
			texSize   = 1.0f / shadowMap[renderTo].Width,
			bias      = 0.0005f,
			depthDistance = 50.0f - 0.01f,
			lightDirection = -sunDir
		};
		shadowBuffer.Set(last);
		last = curr;

		Renderer.RenderTo(shadowMap[renderTo], view, proj, RenderLayer.All &~ RenderLayer.Vfx);
		Renderer.SetGlobalTexture(12, shadowMap[renderFrom]);

		//Mesh.Quad  .Draw(shadowMat, Matrix.TRS(new Vec3(0,-2.5f,0),Quat.LookAt(Vec3.Zero, Vec3.UnitY, Vec3.UnitZ),4));
		//Mesh.Sphere.Draw(shadowMat, Matrix.TS (new Vec3(0,-2,-0.5f), 0.5f));
		model.Draw(Matrix.S(0.1f));

		Mesh.Sphere.Draw(Material.Unlit, Matrix.TS(shadowPos, 0.1f), Color.White, RenderLayer.Vfx);
		Pose pose = view.Pose;
		Lines.Add(shadowPos + pose.Right*size*0.5f, shadowPos - pose.Right*size*0.5f, new Color32(255,200,200,255), 0.01f);
		Lines.Add(shadowPos + pose.Up*size*0.5f, shadowPos - pose.Up*size*0.5f, new Color32(200,255,200,255), 0.01f);
	}
}
