using StereoKit;

namespace StereoKitTest
{
	class DemoPicker : ITest
	{
		Matrix descPose    = Matrix.TR (-0.5f, 0, -0.5f, Quat.LookDir(1,0,1));
		string description = "Applications need to save and load files at runtime! StereoKit has a cross-platform, MR compatible file picker built in, Platform.FilePicker.\n\nOn systems/conditions where a native file picker is available, that's what you'll get! Otherwise, StereoKit will fall back to a custom picker built with StereoKit's UI.";
		Matrix titlePose   = Matrix.TRS(V.XYZ(-0.5f, 0.05f, -0.5f), Quat.LookDir(1, 0, 1), 2);
		string title       = "File Picker";

		Model    model      = null;
		float    modelScale = 1;
		int      modelTask  = 0;
		float    animScrub  = 0;
		float    menuScale  = 1;
		bool     showNodes  = false;
		bool     editNodes  = false;
		bool     showModel  = true;
		Pose     modelPose  = new Pose(0.6f,0,0, Quat.LookDir(-Vec3.Forward));
		Pose     menuPose   = new Pose(0.5f,0,-0.5f, Quat.LookDir(-1,0,1));
		Material volumeMat;
		Material jointMaterial;

		public void Initialize()
		{
			volumeMat = Default.MaterialUIBox.Copy();
			volumeMat["border_size"] = 0.0f;
			volumeMat["border_affect_radius"] = 0.3f;

			jointMaterial = Material.Unlit.Copy();
			jointMaterial.DepthTest   = DepthTest.Always;
			jointMaterial.QueueOffset = 1;
		}
		public void Shutdown() => Platform.FilePickerClose();

		public void Update() {
			UI.WindowBegin("Settings", ref menuPose);

			/// :CodeSample: Platform.FilePicker Platform.FilePickerVisible
			/// ### Opening a Model
			/// This is a simple button that will open a 3D model selection
			/// file picker, and make a call to OnLoadModel after a file has
			/// been successfully picked!
			if (UI.Button("Open Model") && !Platform.FilePickerVisible) {
				Platform.FilePicker(PickerMode.Open, OnLoadModel, null, Assets.ModelFormats);
			}
			/// :End:

			float percent = (Assets.CurrentTask - modelTask) / (float)(Assets.TotalTasks - modelTask);
			UI.ProgressBar(percent);

			UI.Label("Scale");
			UI.HSlider("ScaleSlider", ref menuScale, 0, 1, 0);

			UI.Toggle("Show Nodes", ref showNodes);
			UI.SameLine();
			UI.Toggle("Show Model", ref showModel);
			UI.Toggle("Edit Nodes", ref editNodes);

			if (model != null)
			{
				UI.HSeparator();
				foreach (Anim anim in model.Anims)
				{
					if (UI.Button(anim.Name))
						model.PlayAnim(anim, AnimMode.Loop);
				}
				UI.HSeparator();
				if (UI.HSlider("Scrub", ref animScrub, 0, 1, 0))
				{
					if (model.AnimMode != AnimMode.Manual)
						model.PlayAnim(model.ActiveAnim, AnimMode.Manual);
					model.AnimCompletion = animScrub;
				}
			}

			UI.WindowEnd();

			if (model != null) {
				Bounds scaled = model.Bounds * modelScale * menuScale;
				if (!editNodes)
				{
					UI.HandleBegin("Model", ref modelPose, scaled);
					Default.MeshCube.Draw(volumeMat, Matrix.TS(scaled.center, scaled.dimensions));
					UI.HandleEnd();
				}

				Hierarchy.Push(modelPose.ToMatrix(modelScale * menuScale));
				// Step Animation manually in case showModel is false, and the
				// call to Draw gets skipped.
				model.StepAnim();
				if (showModel) model.Draw(Matrix.Identity);
				if (showNodes) ShowNodes(model, jointMaterial);
				if (editNodes) PickNodes(model, jointMaterial);
				Hierarchy.Pop();
			}

			Text.Add(title, titlePose);
			Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
		}

		/// :CodeSample: Platform.FilePicker Platform.FilePickerVisible
		/// Once you have the filename, it's simply a matter of loading it
		/// from file. This is an example of async loading a model, and 
		/// calculating a scale value that ensures the model is a reasonable
		/// size.
		private void OnLoadModel(string filename)
		{
			model      = Model.FromFile(filename);
			modelTask  = Assets.CurrentTask;
			modelScale = 1 / model.Bounds.dimensions.Magnitude;
			if (model.Anims.Count > 0)
				model.PlayAnim(model.Anims[0], AnimMode.Loop);
		}
		/// :End:

		private static void ShowNodes(Model model, Material jointMaterial)
		{
			float scale = Hierarchy.ToLocalDirection(Vec3.UnitX).Magnitude;
			for (int n = 0; n < model.Nodes.Count; n++)
			{
				var node = model.Nodes[n];
				Mesh.Cube.Draw(jointMaterial, node.ModelTransform.Pose.ToMatrix(0.025f * scale));
				Text.Add(node.Name, Matrix.S(scale)*node.ModelTransform, TextAlign.TopCenter, TextAlign.TopCenter);
			}
		}

		private static void PickNodes(Model model, Material jointMaterial)
		{
			float     scale       = Hierarchy.ToLocalDirection(Vec3.UnitX).Magnitude;
			ModelNode closest     = null;
			float     closestDist = (10 * U.cm * scale) * (10 * U.cm * scale);
			for (int i = 0; i < 2; i += 1) {
				Hand h  = Input.Hand(i);
				Vec3 pt = Hierarchy.ToLocal(h.pinchPt);
				if (!h.IsTracked) continue;
				for (int n = 0; n < model.Nodes.Count; n++)
				{
					var   node = model.Nodes[n];
					float distSq = Vec3.DistanceSq(pt, node.ModelTransform.Translation);
					if (distSq < closestDist)
					{
						closestDist = distSq;
						closest = node;
					}
				}
			}
			if (closest != null)
			{
				Mesh.Sphere.Draw(jointMaterial, closest.ModelTransform.Pose.ToMatrix(0.05f * scale), new Color(0,1,0));
				Pose pose = closest.ModelTransform.Pose;
				if (UI.Handle(closest.Name, ref pose, new Bounds(Vec3.One * 0.1f * scale)))
				{
					closest.ModelTransform = pose.ToMatrix(closest.ModelTransform.Scale);
				}
			}
		}
	}
}
