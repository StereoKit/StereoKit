using StereoKit;

class TestModel : ITest
{
	// Async file loading - one per format
	Model asyncGltf;
	Model asyncStl;
	Model asyncObj;
	bool  gltfOnLoadedFired  = false;
	int   framesSinceRequest = 0;
	int   frameToMeta        = -1;
	int   frameToLoaded      = -1;
	int   frameToCallback    = -1;

	// Procedural model
	Model proceduralModel;

	// Copy-while-loading
	Model asyncSource;
	Model copyWhileLoading;

	// Animated model
	Model animModel;
	bool  animOnLoadedFired = false;

	// Node manipulation model
	Model nodeModel;

	public void Initialize()
	{
		// -- Procedural model: never enters async pipeline --
		proceduralModel = new Model(Mesh.GenerateSphere(0.1f), Material.Default);
		Tests.Test(TestProceduralState);
		Tests.Test(TestProceduralBounds);
		Tests.Test(TestProceduralNodeAccess);
		Tests.Test(TestProceduralDrawReady);

		// -- Async file loads for each format --
		asyncGltf = Model.FromFile("DamagedHelmet.gltf");
		asyncStl  = Model.FromFile("suzanne_bin.stl");
		asyncObj  = Model.FromFile("suzanne.obj");
		asyncGltf.OnLoaded += m => gltfOnLoadedFired = true;

		// Models should not be in None state (that's procedural only)
		Tests.Test(TestAsyncNotNone);

		// Bounds should return 10cm fallback while loading, not block
		Tests.Test(TestBoundsFallbackWhileLoading);

		// Ray intersect should return false while loading, not block
		Tests.Test(TestRayIntersectWhileLoading);

		// Draw during loading should not crash
		asyncGltf.Draw(Matrix.T(0, 0, -1));

		// -- Copy a model that's still loading --
		asyncSource      = Model.FromFile("Radio.glb");
		copyWhileLoading = asyncSource.Copy();
		Tests.Test(TestCopyWhileLoadingState);

		// -- Animated model --
		animModel = Model.FromFile("Cosmonaut.glb");
		animModel.OnLoaded += m => animOnLoadedFired = true;

		// -- Node manipulation on a procedural model --
		nodeModel = new Model();
		Tests.Test(TestEmptyModel);
		Tests.Test(TestNodeAdd);
		Tests.Test(TestNodeSetMesh);
		Tests.Test(TestNodeSetMaterial);
		Tests.Test(TestNodeSetTransform);
		Tests.Test(TestNodeFind);
		Tests.Test(TestNodeHierarchy);
		Tests.Test(TestNodeRemoveVisual);
		Tests.Test(TestNodeVisibility);
		Tests.Test(TestNodeInfo);

		Tests.RunForFrames(60);
	}

	// == Immediate procedural tests ==

	bool TestProceduralState()
	{
		return proceduralModel.AssetState == AssetState.Loaded
			&& proceduralModel.Visuals.Count == 1
			&& proceduralModel.RootNode != null;
	}

	bool TestProceduralBounds()
	{
		return proceduralModel.Bounds.dimensions.Magnitude > 0;
	}

	bool TestProceduralNodeAccess()
	{
		ModelNode root = proceduralModel.RootNode;
		return root != null
			&& root.Mesh     != null
			&& root.Material != null
			&& root.Name     != null
			&& root.Solid    == true
			&& root.Visible  == true;
	}

	bool TestProceduralDrawReady()
	{
		// Procedural models (state None) must not be skipped by the
		// renderer. Verify the mesh has indices ready for drawing.
		Mesh mesh = proceduralModel.RootNode?.Mesh;
		return mesh != null
			&& mesh.IndCount  > 0
			&& mesh.VertCount > 0
			&& mesh.AssetState == AssetState.Loaded;
	}

	// == Immediate async tests ==

	bool TestAsyncNotNone()
	{
		// Async-loaded models should never be in None state; they start
		// at Loading and may have already progressed further by now.
		return asyncGltf.AssetState != AssetState.None
			&& asyncStl .AssetState != AssetState.None
			&& asyncObj .AssetState != AssetState.None;
	}

	bool TestBoundsFallbackWhileLoading()
	{
		if (asyncGltf.AssetState > AssetState.Loading) return true;
		Bounds b = asyncGltf.Bounds;
		return b.dimensions.x == 0.1f
			&& b.dimensions.y == 0.1f
			&& b.dimensions.z == 0.1f;
	}

	bool TestRayIntersectWhileLoading()
	{
		if (asyncGltf.AssetState >= AssetState.Loaded) return true;
		return !asyncGltf.Intersect(new Ray(Vec3.Zero, Vec3.Forward), out _);
	}

	bool TestCopyWhileLoadingState()
	{
		// Copy blocks until the source is loaded, then returns a
		// complete copy.
		return copyWhileLoading.AssetState == AssetState.Loaded
			&& copyWhileLoading.Visuals.Count > 0;
	}

	// == Empty model and node manipulation tests ==

	bool TestEmptyModel()
	{
		return nodeModel.Nodes.Count    == 0
			&& nodeModel.Visuals.Count  == 0
			&& nodeModel.RootNode       == null
			&& nodeModel.Bounds.dimensions.Magnitude == 0;
	}

	bool TestNodeAdd()
	{
		Mesh     mesh = Mesh.GenerateCube(Vec3.One * 0.1f);
		Material mat  = Material.Default;
		nodeModel.AddNode("root", Matrix.Identity, mesh, mat);
		nodeModel.AddNode("empty_node", Matrix.T(1, 0, 0));
		return nodeModel.Nodes.Count   == 2
			&& nodeModel.Visuals.Count == 1;
	}

	bool TestNodeSetMesh()
	{
		ModelNode node = nodeModel.FindNode("root");
		if (node == null) return false;
		Mesh newMesh = Mesh.GenerateSphere(0.05f);
		node.Mesh = newMesh;
		return node.Mesh != null;
	}

	bool TestNodeSetMaterial()
	{
		ModelNode node = nodeModel.FindNode("root");
		if (node == null) return false;
		Material newMat = Material.Default.Copy();
		node.Material = newMat;
		return node.Material != null;
	}

	bool TestNodeSetTransform()
	{
		ModelNode node = nodeModel.FindNode("root");
		if (node == null) return false;
		Matrix t = Matrix.T(0.5f, 0.5f, 0.5f);
		node.ModelTransform = t;
		Vec3 pos = node.ModelTransform.Translation;
		return Vec3.Distance(pos, new Vec3(0.5f, 0.5f, 0.5f)) < 0.001f;
	}

	bool TestNodeFind()
	{
		return nodeModel.FindNode("root")       != null
			&& nodeModel.FindNode("empty_node") != null
			&& nodeModel.FindNode("not_here")   == null;
	}

	bool TestNodeHierarchy()
	{
		ModelNode root = nodeModel.RootNode;
		if (root == null) return false;
		// Two root-level siblings
		return root.Sibling != null;
	}

	bool TestNodeRemoveVisual()
	{
		ModelNode node = nodeModel.FindNode("root");
		if (node == null) return false;
		node.Mesh     = null;
		node.Material = null;
		return node.Mesh == null && node.Material == null;
	}

	bool TestNodeVisibility()
	{
		ModelNode node = nodeModel.FindNode("root");
		if (node == null) return false;
		node.Visible = false;
		bool hidden = node.Visible == false;
		node.Visible = true;
		bool shown = node.Visible == true;
		return hidden && shown;
	}

	bool TestNodeInfo()
	{
		ModelNode node = nodeModel.FindNode("root");
		if (node == null) return false;
		node.SetInfo("test_key", "test_value");
		bool hasVal = node.GetInfo("test_key") == "test_value";
		node.Info.Clear();
		bool cleared = node.GetInfo("test_key") == null;
		return hasVal && cleared;
	}

	// == Deferred async tests (run from Step) ==

	bool TestGltfLoaded()
	{
		return asyncGltf.AssetState    == AssetState.Loaded
			&& asyncGltf.Nodes.Count   >  0
			&& asyncGltf.Visuals.Count >  0
			&& asyncGltf.Bounds.dimensions.Magnitude > 0;
	}

	bool TestStlLoaded()
	{
		return asyncStl.AssetState    == AssetState.Loaded
			&& asyncStl.Visuals.Count == 1
			&& asyncStl.Bounds.dimensions.Magnitude > 0;
	}

	bool TestObjLoaded()
	{
		return asyncObj.AssetState    == AssetState.Loaded
			&& asyncObj.Visuals.Count == 1
			&& asyncObj.Bounds.dimensions.Magnitude > 0;
	}

	bool TestGltfOnLoadedCallback() => gltfOnLoadedFired;

	bool TestAnimModelLoaded()
	{
		return animOnLoadedFired
			&& animModel.AssetState  == AssetState.Loaded
			&& animModel.Anims.Count >  0;
	}

	bool TestAnimPlayAfterLoad()
	{
		if (animModel.Anims.Count <= 0) return false;
		animModel.PlayAnim(animModel.Anims[0], AnimMode.Loop);
		return animModel.ActiveAnim != null;
	}

	bool TestAnimProperties()
	{
		if (animModel.Anims.Count <= 0) return false;
		Anim anim = animModel.Anims[0];
		return anim.Name     != null
			&& anim.Duration >  0;
	}

	bool TestAnimScrub()
	{
		if (animModel.ActiveAnim == null) return false;
		animModel.PlayAnim(animModel.ActiveAnim, AnimMode.Manual);
		animModel.AnimCompletion = 0.5f;
		return animModel.AnimCompletion >= 0.49f
			&& animModel.AnimCompletion <= 0.51f;
	}

	bool TestRayIntersectAfterLoad()
	{
		// Should not crash or block regardless of hit result
		asyncGltf.Intersect(
			new Ray(new Vec3(0, 0, 2), new Vec3(0, 0, -1)),
			out _);
		return true;
	}

	bool TestModelCopyAfterLoad()
	{
		Model copy = asyncGltf.Copy();
		return copy != null
			&& copy.Nodes.Count   == asyncGltf.Nodes.Count
			&& copy.Visuals.Count == asyncGltf.Visuals.Count
			&& copy.Bounds.dimensions.Magnitude > 0;
	}

	bool TestGltfNodeIteration()
	{
		int count = 0;
		ModelNode node = asyncGltf.RootNode;
		while (node != null)
		{
			count++;
			node = node.Child ?? node.Sibling;
		}
		return count > 0;
	}

	bool TestGltfVisualMeshes()
	{
		foreach (var vis in asyncGltf.Visuals)
		{
			if (vis.Mesh     == null) return false;
			if (vis.Material == null) return false;
		}
		return true;
	}

	bool TestStateParityWithProcedural()
	{
		// Procedural and file-loaded models should both report Loaded
		return proceduralModel.AssetState == asyncGltf.AssetState
			&& proceduralModel.AssetState == AssetState.Loaded;
	}

	bool asyncTestsRun = false;
	public void Step()
	{
		framesSinceRequest++;

		// Track state transition timing
		if (frameToMeta    < 0 && asyncGltf.AssetState >= AssetState.LoadedMeta)
			frameToMeta = framesSinceRequest;
		if (frameToLoaded  < 0 && asyncGltf.AssetState >= AssetState.Loaded)
			frameToLoaded = framesSinceRequest;
		if (frameToCallback < 0 && gltfOnLoadedFired)
			frameToCallback = framesSinceRequest;

		// Keep the test alive until async loads complete
		if (!asyncTestsRun)
			Tests.RunContinue();

		// Once all formats are loaded, run the full test suite
		if (!asyncTestsRun &&
			asyncGltf .AssetState >= AssetState.Loaded &&
			asyncStl  .AssetState >= AssetState.Loaded &&
			asyncObj  .AssetState >= AssetState.Loaded &&
			animModel .AssetState >= AssetState.Loaded)
		{
			asyncTestsRun = true;
			Log.Info($"Async model load: LoadedMeta at frame {frameToMeta}, Loaded at frame {frameToLoaded}, OnLoaded at frame {frameToCallback}");
			Tests.Screenshot("Tests/ModelAsync.jpg", Tests.SceneFrame, 600, 400, 90, new Vec3(0, 0, -0.35f), new Vec3(0, 0, -0.5f));

			Tests.Test(TestGltfLoaded);
			Tests.Test(TestStlLoaded);
			Tests.Test(TestObjLoaded);
			Tests.Test(TestGltfOnLoadedCallback);
			Tests.Test(TestAnimModelLoaded);
			Tests.Test(TestAnimPlayAfterLoad);
			Tests.Test(TestAnimProperties);
			Tests.Test(TestAnimScrub);
			Tests.Test(TestRayIntersectAfterLoad);
			Tests.Test(TestModelCopyAfterLoad);
			Tests.Test(TestGltfNodeIteration);
			Tests.Test(TestGltfVisualMeshes);
			Tests.Test(TestStateParityWithProcedural);
		}

		// Draw every frame to stress the render path through all states
		float s = 0.15f;
		asyncGltf      .Draw(Matrix.TS(-s*2, 0, -0.5f, 0.1f));
		asyncStl       .Draw(Matrix.TS(-s,   0, -0.5f, 0.1f));
		asyncObj       .Draw(Matrix.TS( 0,   0, -0.5f, 0.1f));
		animModel      .Draw(Matrix.TS( s,   0, -0.5f, 0.1f));
		proceduralModel.Draw(Matrix.TS( s*2, 0, -0.5f, 0.1f));
	}

	public void Shutdown()
	{
		if (!asyncTestsRun)
			Tests.Fail("TestModel: async tests never ran! Models may not have finished loading in time.");
	}
}
