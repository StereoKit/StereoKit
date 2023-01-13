using StereoKit;
using System;
using System.Threading.Tasks;

class TestConcurrentSetColors : ITest
{
	static bool run = false;
	Material[] materials = null;

	public void Initialize()
	{
		run       = true;
		materials = new Material[4];
		for (int i = 0; i < materials.Length; i++)
			materials[i] = Material.Default.Copy();
		Task.Run(() => LoadColors(materials[0], TexType.Dynamic | TexType.ImageNomips));
		Task.Run(() => LoadColors(materials[1], TexType.ImageNomips));
		Task.Run(() => LoadColors(materials[2], TexType.Dynamic | TexType.ImageNomips));
		Task.Run(() => LoadColors(materials[3], TexType.ImageNomips));

		Tests.RunForSeconds(1);
	}

	static void LoadColors(Material mat, TexType type)
	{
		Tex       tex    = new Tex(type);
		int       size   = 128;
		Color32[] colors = new Color32[size * size];
		mat[MatParamName.DiffuseTex] = tex;
		Random r = new Random();
		while (run)
		{
			for (int i = 0; i < colors.Length; i++)
			{
				colors[i].r = (byte)(r.Next() % 255);
				colors[i].g = (byte)(r.Next() % 255);
				colors[i].b = (byte)(r.Next() % 255);
				colors[i].a = 255;
			}
			tex.SetColors(size, size, colors);
		}
	}

	public void Shutdown()
	{
		run = false;
	}

	public void Update()
	{
		for (int i = 0; i < materials.Length; i++) {
			Mesh.Sphere.Draw(materials[i], Matrix.TS(V.X0Z((i-materials.Length/2+0.5f)*0.25f, -0.5f), 0.25f));
		}
	}
}