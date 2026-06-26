using StereoKit;

class TestMesh : ITest
{
	Mesh meshEmpty;
	Mesh meshSync;
	Mesh meshAsync;
	Mesh meshAsyncBounds;
	Mesh meshGenerated;
	bool onLoadedFired   = false;
	bool asyncChecked    = false;
	int  framesSinceRequest = 0;
	int  frameToLoaded      = -1;
	int  frameToCallback    = -1;

	static Vertex[] MakeQuadVerts() => new Vertex[] {
		new Vertex(new Vec3(-0.5f, -0.5f, 0), new Vec3(0, 0, -1)),
		new Vertex(new Vec3( 0.5f, -0.5f, 0), new Vec3(0, 0, -1)),
		new Vertex(new Vec3( 0.5f,  0.5f, 0), new Vec3(0, 0, -1)),
		new Vertex(new Vec3(-0.5f,  0.5f, 0), new Vec3(0, 0, -1)),
	};
	static uint[] MakeQuadInds() => new uint[] { 2, 1, 0, 3, 2, 0 };

	public void Initialize()
	{
		meshEmpty = new Mesh();

		meshSync = new Mesh(MakeQuadVerts(), MakeQuadInds());

		// Async upload — no BlockForPriority so we can measure real
		// frame timing in Step.
		meshAsync = new Mesh(MakeQuadVerts(), MakeQuadInds(), MeshData.Async);
		meshAsync.OnLoaded += m => onLoadedFired = true;

		meshAsyncBounds = new Mesh(MakeQuadVerts(), MakeQuadInds(), MeshData.CalcBounds | MeshData.Async);

		meshGenerated = Mesh.GenerateSphere(0.5f);

		// Synchronous tests can run immediately
		Tests.Test(TestEmptyMeshState);
		Tests.Test(TestSyncUpload);
		Tests.Test(TestGeneratedMeshState);
	}

	bool TestEmptyMeshState()
	{
		if (meshEmpty.AssetState != AssetState.None)
			return false;
		if (meshEmpty.IndCount != 0)
			return false;
		return true;
	}

	bool TestSyncUpload()
	{
		if (meshSync.AssetState != AssetState.Loaded)
			return false;
		if (meshSync.VertCount != 4)
			return false;
		if (meshSync.IndCount != 6)
			return false;
		if (meshSync.Bounds.dimensions.x <= 0)
			return false;
		return true;
	}

	bool TestGeneratedMeshState()
	{
		if (meshGenerated.AssetState != AssetState.Loaded)
			return false;
		if (meshGenerated.VertCount <= 0)
			return false;
		return true;
	}

	bool TestAsyncUpload()
	{
		if (meshAsync.AssetState != AssetState.Loaded)
			return false;
		if (meshAsync.VertCount != 4)
			return false;
		if (meshAsync.IndCount != 6)
			return false;
		return true;
	}

	bool TestAsyncBounds()
	{
		if (meshAsyncBounds.AssetState != AssetState.Loaded)
			return false;
		if (meshAsyncBounds.Bounds.dimensions.x <= 0)
			return false;
		return true;
	}

	bool TestOnLoadedCallback() => onLoadedFired;

	public void Step()
	{
		framesSinceRequest++;
		if (frameToLoaded < 0 && meshAsync.AssetState == AssetState.Loaded)
			frameToLoaded = framesSinceRequest;
		if (frameToCallback < 0 && onLoadedFired)
			frameToCallback = framesSinceRequest;

		// Keep the test alive until async loads complete
		if (!asyncChecked)
			Tests.RunContinue();

		// Once the callback has fired, run all async tests and log timing
		if (!asyncChecked && frameToCallback >= 0) {
			asyncChecked = true;
			Log.Info($"Async mesh upload: AssetState.Loaded at frame {frameToLoaded}, OnLoaded callback at frame {frameToCallback}");
			Tests.Test(TestAsyncUpload);
			Tests.Test(TestAsyncBounds);
			Tests.Test(TestOnLoadedCallback);
		}

		float spacing = 0.15f;
		Quat facing = Quat.LookDir(0, 0, 1);
		meshEmpty       .Draw(Default.Material, Matrix.TR(-spacing*2, 0, -0.5f, facing));
		meshSync        .Draw(Default.Material, Matrix.TRS(V.XYZ(-spacing, 0, -0.5f), facing, 0.1f));
		meshAsync       .Draw(Default.Material, Matrix.TRS(V.XYZ(0, 0, -0.5f), facing, 0.1f));
		meshAsyncBounds .Draw(Default.Material, Matrix.TRS(V.XYZ(spacing, 0, -0.5f), facing, 0.1f));
		meshGenerated   .Draw(Default.Material, Matrix.TRS(V.XYZ(spacing*2, 0, -0.5f), facing, 0.1f));
	}

	public void Shutdown()
	{
		if (!asyncChecked)
			Tests.Fail("TestMesh: async tests never ran! Mesh may not have finished loading in time.");
	}
}
