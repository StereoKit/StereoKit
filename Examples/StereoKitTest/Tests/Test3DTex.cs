using StereoKit;
using System.Runtime.InteropServices;

class Test3DTex : ITest
{
	const  int     Size = 32;
	static Vec3    Pos  = V.XYZ(0, 0, -0.5f);
	static float   Sz   = 0.4f;

	Tex      _volume;
	Material _material;

	public void Initialize()
	{
		_volume = new Tex(TexType.ImageNomips | TexType.Volume, TexFormat.Rgba32);

		// Three colored spheres packed into a 32^3 RGBA8 volume. Solid
		// inside the radius, fully transparent outside, so the raymarch
		// has clear opaque blobs to render.
		Color32[] data = BuildSphereVolume(Size);
		var handle = GCHandle.Alloc(data, GCHandleType.Pinned);
		try   { _volume.SetColors(Size, Size, Size, handle.AddrOfPinnedObject()); }
		finally { handle.Free(); }
		_volume.AddressMode = TexAddress.Clamp;

		_material = new Material(Shader.FromFile("texture3d.hlsl"));
		_material.SetTexture("volume", _volume);
		_material.Transparency = Transparency.Blend;

		// world_inv is the inverse of the cube's TRS, so the shader can
		// transform the camera into cube-local space for raymarching.
		// Transpose to match HLSL's column-major matrix packing (same as
		// TestMatrixShaderParam.cs does for custom_transform).
		Matrix world = Matrix.TS(Pos, Sz);
		_material.SetMatrix("world_inv", world.Inverse.Transposed);
	}
	public void Shutdown() { }

	public void Step()
	{
		Tests.Screenshot("Tests/Texture3D.jpg", 600, 600, 50, V.XYZ(0.4f, 0.3f, -0.05f), Pos);
		Mesh.Cube.Draw(_material, Matrix.TS(Pos, Sz));
	}

	static Color32[] BuildSphereVolume(int size)
	{
		Color32[] data = new Color32[size*size*size];
		Vec3[]    centers = { V.XYZ(0.3f, 0.55f, 0.5f), V.XYZ(0.5f, 0.3f, 0.5f), V.XYZ(0.7f, 0.3f, 0.7f) };
		float[]   radii   = { 0.3f, 0.2f, 0.25f };
		Color32[] colors  = {
			new Color32(255,  64,  64, 255), // red
			new Color32( 64, 255,  64, 255), // green
			new Color32( 64,  64, 255, 255), // blue
		};

		for (int z = 0; z < size; z++)
		for (int y = 0; y < size; y++)
		for (int x = 0; x < size; x++)
		{
			Vec3 p = V.XYZ((x+0.5f)/size, (y+0.5f)/size, (z+0.5f)/size);
			Color32 c = new Color32(0,0,0,0);
			for (int i = 0; i < 3; i++)
			{
				if (Vec3.Distance(p, centers[i]) < radii[i]) { c = colors[i]; break; }
			}
			data[x + y*size + z*size*size] = c;
		}
		return data;
	}
}
