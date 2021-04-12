using StereoKit;
using System;

class DemoRenderTexture : ITest
{
	Tex      renderSurface;
	Material renderView;
	public void Initialize()
	{
		renderSurface = new Tex(TexType.Rendertarget, TexFormat.Rgba32);
		renderSurface.SetSize(512, 512);
		renderSurface.AddZBuffer(TexFormat.Depth32);
		renderView = Default.MaterialUnlit.Copy();
		renderView[MatParamName.DiffuseTex] = renderSurface;
		renderView.FaceCull = Cull.None;
	}

	Pose from = new Pose(V.XYZ(0,0,-0.2f), Quat.Identity);
	Pose at   = new Pose(V.XYZ(0,0,-0.3f), Quat.Identity);
	public void Update()
	{
		UI.Handle("from", ref from, new Bounds(Vec3.One*0.02f), true);
		UI.Handle("to",   ref at,   new Bounds(Vec3.One*0.02f), true);
		Lines.Add(from.position, at.position, Color.White, 0.005f);
		from.orientation = at.orientation = Quat.LookDir(at.position-from.position);
		Default.MeshQuad.Draw(renderView, Matrix.TRS(at.position + at.orientation*Vec3.Up*0.06f, Quat.LookDir(from.position-at.position), 0.1f));

		Renderer.RenderTo(renderSurface, 
			Matrix.TR(at.position+at.Forward*0.01f, at.orientation), 
			Matrix.Perspective(10 + Math.Max(0,Math.Min(1,(Vec3.Distance(from.position, at.position)-0.05f)/0.2f))*110, 1, 0.01f, 100));
	}

	public void Shutdown() { }
}