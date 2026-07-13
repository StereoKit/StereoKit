using StereoKit;
using System;

// Exercises the CPU skinning surface on Mesh: SetSkin / UpdateSkin /
// HasSkin / Copy. Builds a unit cube whose 8 verts are rigidly bound
// to a single bone, then Copies the source mesh and drives two
// instances with different bone palettes. The two copies must deform
// independently — mesh_update_skin mutates the target mesh's vertex
// buffer in place, so this only works if Copy produced truly
// independent skin state.
class TestMeshSkin : ITest
{
	Mesh _src;
	Mesh _a;
	Mesh _b;

	public void Initialize()
	{
		_src = Mesh.GenerateCube(Vec3.One * 0.1f);

		// All verts rigidly attached to bone 0.
		ushort[] boneIds = new ushort[_src.VertCount * 4];
		Vec4[]   weights = new Vec4[_src.VertCount];
		for (int i = 0; i < _src.VertCount; i++)
			weights[i] = new Vec4(1, 0, 0, 0);
		Matrix[] restingTransforms = new[] { Matrix.Identity };

		_src.SetSkin(boneIds, weights, restingTransforms);

		_a = _src.Copy();
		_b = _src.Copy();

		Tests.Test(TestHasSkin);
		Tests.Test(TestCopyPreservesSkin);
		Tests.Test(TestCopyInstancesDeformIndependently);
	}

	public void Shutdown() { }

	public void Step()
	{
		// Bob each instance with a different palette so the screenshot
		// shows independent deformation.
		Matrix[] paletteA = new[] { Matrix.T(0,  SKMath.Sin(Time.Totalf * 2)        * 0.05f, 0) };
		Matrix[] paletteB = new[] { Matrix.T(0,  SKMath.Sin(Time.Totalf * 2 + 1.57f) * 0.05f, 0) };
		_a.UpdateSkin(paletteA);
		_b.UpdateSkin(paletteB);

		_a.Draw(Material.Default, Matrix.T(-0.1f, 0, -0.5f));
		_b.Draw(Material.Default, Matrix.T( 0.1f, 0, -0.5f));
	}

	bool TestHasSkin()
	{
		return _src.HasSkin && _a.HasSkin && _b.HasSkin;
	}

	bool TestCopyPreservesSkin()
	{
		// Copy must carry skin data forward — without this, UpdateSkin
		// on the copy would be a no-op.
		return _a.HasSkin && _b.HasSkin;
	}

	bool TestCopyInstancesDeformIndependently()
	{
		// Drive the two copies with palettes that translate bone 0 in
		// opposite directions. UpdateSkin recomputes bounds from the
		// deformed verts, so the resulting bounds centers must differ.
		_a.UpdateSkin(new[] { Matrix.T(0,  0.5f, 0) });
		_b.UpdateSkin(new[] { Matrix.T(0, -0.5f, 0) });
		Bounds ba = _a.Bounds;
		Bounds bb = _b.Bounds;
		if (ba.center.y < 0.4f) return false;
		if (bb.center.y > -0.4f) return false;
		return true;
	}
}
