using StereoKit;
using System;
using System.Runtime.InteropServices;

class TestVertexFormat : ITest
{
	/// :CodeSample: Mesh.SetVerts VertComponentAttribute VertSemantic VertFmt
	/// ### Custom vertex formats
	/// Meshes can use custom vertex layouts! Define a struct that provides
	/// what your shader's vertex stage needs, and tag each field with a
	/// VertComponent attribute saying what it is. StereoKit derives the
	/// vertex format from the struct automatically. Since vertex data is
	/// matched to shader inputs by semantic, fields don't need to be in the
	/// same order as the shader declares them!
	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	struct VertPosColor
	{
		[VertComponent(VertSemantic.Color,    VertFmt.U8Normalized, 4)]
		public Color32 color;
		[VertComponent(VertSemantic.Position, VertFmt.F32,          3)]
		public Vec3    pos;

		public VertPosColor(Vec3 position, Color32 c) { pos = position; color = c; }
	}

	Mesh     _mesh;
	Material _material;

	Mesh BuildOctahedron(float s)
	{
		Mesh mesh = new Mesh();
		mesh.SetVerts(new VertPosColor[] {
			new VertPosColor(V.XYZ( s, 0, 0), new Color32(255,  0,  0,255)),
			new VertPosColor(V.XYZ(-s, 0, 0), new Color32(  0,255,  0,255)),
			new VertPosColor(V.XYZ( 0, s, 0), new Color32(  0,  0,255,255)),
			new VertPosColor(V.XYZ( 0,-s, 0), new Color32(255,255,  0,255)),
			new VertPosColor(V.XYZ( 0, 0, s), new Color32(  0,255,255,255)),
			new VertPosColor(V.XYZ( 0, 0,-s), new Color32(255,  0,255,255)) });
		mesh.SetInds(new uint[] {
			2,4,0,  2,0,5,  2,5,1,  2,1,4,
			3,0,4,  3,5,0,  3,1,5,  3,4,1 });
		return mesh;
	}
	/// :End:

	public void Initialize()
	{
		_mesh     = BuildOctahedron(0.1f);
		_material = new Material(Shader.FromFile("Shaders/vert_custom.hlsl"));

		Tests.Test(TestBoundsFromPosition);
		Tests.Test(TestRoundTrip);
		Tests.Test(TestLayoutValidation);
		Tests.Test(TestSetData);
		Tests.Test(TestSetDataAsync);
		Tests.Test(TestGetTriangle);
		Tests.Test(TestBvhIntersect);
		Tests.Test(TestCalculatedNormals);
		Tests.Test(TestSkinnedCustomFormat);
		Tests.Test(TestVertexAsGeneric);
		Tests.Test(TestCreativeUnaligned);
		Tests.Test(TestCreativeRoundTrip);
	}

	// GetTriangle decodes custom formats into Vertex, with defaults for
	// components the format doesn't have.
	bool TestGetTriangle()
	{
		if (!_mesh.GetTriangle(0, out Vertex a, out Vertex b, out Vertex c))
			return false;

		// Triangle 0 is inds 2,4,0 of the octahedron
		if (Vec3.DistanceSq(a.pos, V.XYZ(0, 0.1f, 0)) > 1e-10f) return false;
		if (Vec3.DistanceSq(b.pos, V.XYZ(0, 0, 0.1f)) > 1e-10f) return false;
		if (Vec3.DistanceSq(c.pos, V.XYZ(0.1f, 0, 0)) > 1e-10f) return false;
		// Normals aren't in the format, they read as the (0,1,0) default
		if (Vec3.DistanceSq(a.norm, V.XYZ(0, 1, 0)) > 1e-10f)   return false;
		// Colors decode, vert 2 is blue
		return a.col.r == 0 && a.col.g == 0 && a.col.b == 255 && a.col.a == 255;
	}

	// The BVH intersection path builds from format-aware collision data.
	bool TestBvhIntersect()
	{
		Ray ray = new Ray(V.XYZ(0.01f, 0.01f, -1), V.XYZ(0, 0, 1));
		if (!_mesh.Intersect(ray, out Ray at, out uint startInds))
			return false;
		// The -z face plane satisfies x+y-z = s, so the hit lands at 0.02-s
		return Math.Abs(at.position.z - (0.02f - 0.1f)) < 0.0001f;
	}

	// The built-in Vertex type works with the generic APIs too, and lands
	// on the same default format as the typed overloads.
	bool TestVertexAsGeneric()
	{
		Vertex[] verts = {
			new Vertex(V.XYZ(-1, 0, 0), Vec3.Up),
			new Vertex(V.XYZ( 1, 0, 0), Vec3.Up),
			new Vertex(V.XYZ( 0, 1, 0), Vec3.Up) };
		Mesh mesh = new Mesh();
		mesh.SetVerts<Vertex>(verts);
		mesh.SetInds(new uint[] { 0, 1, 2 });

		Vertex[] back = mesh.GetVerts<Vertex>();
		if (back == null || back.Length != 3)                        return false;
		if (Vec3.DistanceSq(back[2].pos, V.XYZ(0, 1, 0)) > 1e-10f)   return false;
		// The typed getter agrees, it's the same default format mesh
		return mesh.GetVerts() != null;
	}

	// CPU skinning deforms positions through the vertex format, and a
	// skinned mesh must reject vertex format changes.
	bool TestSkinnedCustomFormat()
	{
		VertPosColor[] verts = _mesh.GetVerts<VertPosColor>();
		uint[]         inds  = _mesh.GetInds ();
		Mesh mesh = new Mesh();
		mesh.SetData(verts, inds);

		// Every vertex fully weighted to a single identity-rest bone
		ushort[] ids     = new ushort[verts.Length * 4];
		Vec4[]   weights = new Vec4  [verts.Length];
		for (int i = 0; i < verts.Length; i++) weights[i] = V.XYZW(1, 0, 0, 0);
		mesh.SetSkin(ids, weights, new Matrix[] { Matrix.Identity });

		// Moving the bone moves the deformed bounds with it
		mesh.UpdateSkin(new Matrix[] { Matrix.T(1, 0, 0) });
		if (Vec3.DistanceSq(mesh.Bounds.center, V.XYZ(1, 0, 0)) > 1e-6f)
			return false;

		// A format change on a skinned mesh gets rejected, the mesh should
		// still be readable with its original format.
		Log.Warn("Expected error:");
		mesh.SetVerts(new Vertex[] { new Vertex(Vec3.Zero, Vec3.Up) });
		VertPosColor[] after = mesh.GetVerts<VertPosColor>();
		return after != null && after.Length == verts.Length;
	}

	// A normal-less obj sends the loader through calculate normals, which
	// routes through the vertex format codec.
	bool TestCalculatedNormals()
	{
		string obj =
			"v 0 0 0\n" +
			"v 1 0 0\n" +
			"v 1 1 0\n" +
			"v 0 1 0\n" +
			"f 1 2 3\n" +
			"f 1 3 4\n";
		Model model = Model.FromMemory("flat_quad.obj", System.Text.Encoding.UTF8.GetBytes(obj));
		Vertex[] verts = model.Visuals[0].Mesh.GetVerts();
		if (verts == null || verts.Length == 0) return false;

		// Every vertex of a flat +Z facing quad gets a (0,0,1) normal
		foreach (Vertex v in verts)
			if (Vec3.DistanceSq(v.norm, V.XYZ(0, 0, 1)) > 1e-6f) return false;
		return true;
	}

	bool TestBoundsFromPosition()
		=> Vec3.DistanceSq(_mesh.Bounds.dimensions, V.XYZ(0.2f, 0.2f, 0.2f)) < 0.0001f;

	bool TestRoundTrip()
	{
		VertPosColor[] verts = _mesh.GetVerts<VertPosColor>();
		if (verts == null || verts.Length != 6)                        return false;
		if (Vec3.DistanceSq(verts[0].pos, V.XYZ(0.1f, 0, 0)) > 1e-8f)  return false;
		if (verts[5].color.a != 255 || verts[5].color.g != 0)          return false;

		// A struct whose format doesn't match the mesh must throw rather
		// than misinterpret the data.
		try { _mesh.GetVerts<VertPosOnly>(); return false; }
		catch (InvalidOperationException) { }

		return true;
	}

	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	struct VertPosOnly
	{
		[VertComponent(VertSemantic.Position, VertFmt.F32, 3)]
		public Vec3 pos;
	}

	// These structs are intentionally broken to check layout validation,
	// their fields are never assigned.
	#pragma warning disable 0649
	struct VertPadded
	{
		[VertComponent(VertSemantic.Color, VertFmt.U8Normalized, 1)]
		public byte  color;
		// Default layout pads 3 bytes here, which the layout reflection
		// must catch and complain about.
		[VertComponent(VertSemantic.Position, VertFmt.F32, 1)]
		public float pos;
	}

	struct VertUntagged
	{
		public Vec3 pos;
	}

	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	struct VertWrongSize
	{
		[VertComponent(VertSemantic.Position, VertFmt.F32, 2)]
		public Vec3 pos;
	}
	#pragma warning restore 0649

	bool TestLayoutValidation()
	{
		Mesh mesh = new Mesh();

		try { mesh.SetVerts(new VertPadded   [1]); return false; } catch (ArgumentException) { }
		try { mesh.SetVerts(new VertUntagged [1]); return false; } catch (ArgumentException) { }
		try { mesh.SetVerts(new VertWrongSize[1]); return false; } catch (ArgumentException) { }

		return true;
	}

	static bool MatchesReference(Mesh mesh, Mesh reference)
	{
		if (Vec3.DistanceSq(mesh.Bounds.dimensions, reference.Bounds.dimensions) > 1e-8f)
			return false;

		VertPosColor[] a = mesh     .GetVerts<VertPosColor>();
		VertPosColor[] b = reference.GetVerts<VertPosColor>();
		if (a == null || b == null || a.Length != b.Length) return false;
		for (int i = 0; i < a.Length; i++)
		{
			if (Vec3.DistanceSq(a[i].pos, b[i].pos) > 1e-10f) return false;
			if (a[i].color.r != b[i].color.r || a[i].color.g != b[i].color.g ||
				a[i].color.b != b[i].color.b || a[i].color.a != b[i].color.a) return false;
		}
		return mesh.GetInds().Length == reference.GetInds().Length;
	}

	// SetData with a custom format should produce the same mesh SetVerts +
	// SetInds does.
	bool TestSetData()
	{
		VertPosColor[] verts = _mesh.GetVerts<VertPosColor>();
		uint[]         inds  = _mesh.GetInds ();

		Mesh mesh = new Mesh();
		mesh.SetData(verts, inds);
		return MatchesReference(mesh, _mesh);
	}

	bool TestSetDataAsync()
	{
		VertPosColor[] verts = _mesh.GetVerts<VertPosColor>();
		uint[]         inds  = _mesh.GetInds ();

		Mesh mesh = new Mesh();
		mesh.SetData(verts, inds, MeshData.CalcBounds | MeshData.Async);
		Assets.BlockForPriority(0);
		return MatchesReference(mesh, _mesh);
	}

	// A deliberately awkward "compressed" vertex format that stresses the
	// trickier paths all at once: a leading 1-byte component pushes the
	// float3 position to byte offset 1, so bounds, collision, and the BVH
	// all have to read it unaligned; the normal and UV are bit-packed into
	// plain integer fields (the format describes the bytes, not the field's
	// C# type); and the components are declared out of the shader's input
	// order. Five components, none of them where you'd expect.
	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	struct VertCreative
	{
		[VertComponent(VertSemantic.Psize,    VertFmt.U8Normalized, 1)] public byte    size;   // @0
		[VertComponent(VertSemantic.Position, VertFmt.F32,          3)] public Vec3    pos;    // @1  (unaligned!)
		[VertComponent(VertSemantic.Color,    VertFmt.U8Normalized, 4)] public Color32 color;  // @13
		[VertComponent(VertSemantic.Normal,   VertFmt.I8Normalized, 4)] public int     normal; // @17 4x i8, packed
		[VertComponent(VertSemantic.Texcoord, VertFmt.F16,          2)] public uint    uv;     // @21 2x f16, packed
	}

	static byte ToI8Norm (float v)          => (byte)(sbyte)Math.Round(Math.Clamp(v, -1f, 1f) * 127f);
	static int  PackNormal(Vec3  n)         => ToI8Norm(n.x) | (ToI8Norm(n.y) << 8) | (ToI8Norm(n.z) << 16); // w stays 0
	static uint PackUv    (float u, float v) => (uint)BitConverter.HalfToUInt16Bits((Half)u) | ((uint)BitConverter.HalfToUInt16Bits((Half)v) << 16);

	Mesh BuildCreativeOctahedron(float s)
	{
		Vec3[]    p = { V.XYZ(s,0,0), V.XYZ(-s,0,0), V.XYZ(0,s,0), V.XYZ(0,-s,0), V.XYZ(0,0,s), V.XYZ(0,0,-s) };
		Color32[] c = { new Color32(255,0,0,255), new Color32(0,255,0,255), new Color32(0,0,255,255),
		                new Color32(255,255,0,255), new Color32(0,255,255,255), new Color32(255,0,255,255) };

		VertCreative[] verts = new VertCreative[6];
		for (int i = 0; i < 6; i++)
			verts[i] = new VertCreative {
				size   = 128,
				pos    = p[i],
				color  = c[i],
				normal = PackNormal(p[i].Normalized),
				uv     = PackUv(i / 5.0f, 0) };

		Mesh mesh = new Mesh();
		mesh.SetVerts(verts);
		mesh.SetInds(new uint[] {
			2,4,0,  2,0,5,  2,5,1,  2,1,4,
			3,0,4,  3,5,0,  3,1,5,  3,4,1 });
		return mesh;
	}

	// The creative format's position sits at an unaligned byte offset, so
	// this drives the unaligned reads in bounds, collision, and the BVH,
	// plus a GetTriangle decode of every packed component.
	bool TestCreativeUnaligned()
	{
		Mesh mesh = BuildCreativeOctahedron(0.1f);

		// Bounds are computed from the (unaligned) position component.
		if (Vec3.DistanceSq(mesh.Bounds.dimensions, V.XYZ(0.2f, 0.2f, 0.2f)) > 0.0001f) return false;

		// Ray intersection runs through format-aware, unaligned collision data.
		Ray ray = new Ray(V.XYZ(0.01f, 0.01f, -1), V.XYZ(0, 0, 1));
		if (!mesh.Intersect(ray, out Ray at, out _)) return false;
		if (Math.Abs(at.position.z - (0.02f - 0.1f)) > 0.0001f) return false;

		// GetTriangle decodes every component, incl. the packed i8 normal.
		// Triangle 0 is inds 2,4,0; vert 0 is the +x corner (red, normal +x).
		if (!mesh.GetTriangle(0, out Vertex a, out _, out Vertex c)) return false;
		if (Vec3.DistanceSq(c.pos,  V.XYZ(0.1f, 0, 0)) > 1e-10f) return false;
		if (Vec3.DistanceSq(c.norm, V.XYZ(1, 0, 0))    > 0.02f)  return false; // not the (0,1,0) default
		if (c.col.r != 255 || c.col.g != 0)                     return false;
		// Vert 2 is the +y corner, blue.
		if (a.col.b != 255 || a.col.r != 0)                     return false;

		return true;
	}

	// A five-component packed format round-trips byte-for-byte through
	// GetVerts, integer fields included.
	bool TestCreativeRoundTrip()
	{
		Mesh           mesh  = BuildCreativeOctahedron(0.1f);
		VertCreative[] verts = mesh.GetVerts<VertCreative>();
		if (verts == null || verts.Length != 6) return false;

		VertCreative got = verts[0]; // the +x corner
		return got.size == 128
			&& Vec3.DistanceSq(got.pos, V.XYZ(0.1f, 0, 0)) < 1e-12f
			&& got.color.r == 255 && got.color.g == 0 && got.color.b == 0 && got.color.a == 255
			&& got.normal == PackNormal(V.XYZ(1, 0, 0))
			&& got.uv     == PackUv(0, 0);
	}

	public void Shutdown() { }

	public void Step()
	{
		_mesh.Draw(_material, Matrix.R(0, 45, 0));

		Tests.Screenshot("Tests/VertexFormat.jpg", 400, 400, 90, V.XYZ(0.12f, 0.12f, 0.25f), V.XYZ(0, 0, 0));
	}
}
