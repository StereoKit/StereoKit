using StereoKit;
using System;
using System.Linq;

namespace StereoKitTest.Demos
{
	public class PointCloud
	{
		Material mat;
		Vertex[] verts;
		Mesh     mesh;

		bool  distanceIndependant;
		float pointSize;

		public bool  DistanceIndependantSize { get => distanceIndependant; set { mat["screen_size"] = value?1.0f:0.0f; distanceIndependant = value; } }
		public float PointSize               { get => pointSize;           set { mat["point_size" ] = value;           pointSize           = value; } }

		public PointCloud(float pointSizeMeters = 0.01f)
		{
			mat = new Material(Shader.FromFile("point_cloud.hlsl"));
			PointSize = pointSizeMeters;
		}
		public PointCloud(float pointSizeMeters, Mesh fromMesh)
			: this (pointSizeMeters) => SetPoints(fromMesh);
		public PointCloud(float pointSizeMeters, Vertex[] fromVerts)
			: this(pointSizeMeters) => SetPoints(fromVerts);
		public PointCloud(float pointSizeMeters, Model fromModel)
			: this(pointSizeMeters) => SetPoints(fromModel);

		public void Draw(Matrix transform) => mesh?.Draw(mat, transform);

		public void SetPoints(Vertex[] points)
		{
			if (verts == null)
				verts = new Vertex[points.Length*4];
			if (verts.Length != points.Length * 4)
			{
				Log.Err("You can re-use a point cloud, but  the number of points should stay the same!");
				return;
			}

			for (int i = 0; i < points.Length; i++)
			{
				Vertex p = points[i];
				int idx = i*4;
				verts[idx  ] = new Vertex(p.pos, p.norm, V.XY(-.5f, .5f), p.col);
				verts[idx+1] = new Vertex(p.pos, p.norm, V.XY( .5f, .5f), p.col);
				verts[idx+2] = new Vertex(p.pos, p.norm, V.XY( .5f,-.5f), p.col);
				verts[idx+3] = new Vertex(p.pos, p.norm, V.XY(-.5f,-.5f), p.col);
			}
			if (mesh == null) {
				uint[] inds = new uint[points.Length*6];
				for (uint i = 0; i < points.Length; i++)
				{
					uint ind  = i*6;
					uint vert = i*4;
					inds[ind  ] = vert+2;
					inds[ind+1] = vert+1;
					inds[ind+2] = vert+0;

					inds[ind+3] = vert+3;
					inds[ind+4] = vert+2;
					inds[ind+5] = vert+0;
				}
				mesh = new Mesh();
				mesh.SetInds(inds);
			}
			mesh.SetVerts(verts);
		}
		public void SetPoints(Mesh fromMeshVerts)
			=> SetPoints(fromMeshVerts.GetVerts());
		public void SetPoints(Model fromModelVerts)
			=> SetPoints(fromModelVerts.Visuals
				.SelectMany(v => TransformVerts(v.ModelTransform, v.Mesh.GetVerts()))
				.ToArray   ());

		private static Vertex[] TransformVerts(Matrix mat, Vertex[] verts)
		{
			for (int i = 0; i < verts.Length; i++)
				verts[i].pos = mat.Transform(verts[i].pos);
			return verts;
		}
	}

	class DemoPointCloud : ITest
	{
		Matrix     descPose     = Matrix.TR (-0.5f, 0, -0.5f, Quat.LookDir(1,0,1));
		string     description  = "Point clouds are not a built-in feature of StereoKit, but it's not hard to do this yourself! Check out the code for this demo for a class that'll help you do this directly from data, or from a Model.";
		Matrix     titlePose    = Matrix.TRS(V.XYZ(-0.5f, 0.05f, -0.5f), Quat.LookDir(1, 0, 1), 2);
		string     title        = "Point Clouds";

		Pose       cloudPose    = new Pose(0.5f, 0, -0.5f, Quat.LookDir(-1, 0, 1));
		float      cloudScale   = 1;
		PointCloud cloud;

		Pose       settingsPose = new Pose(0.8f, 0.05f, -0.2f, Quat.LookDir(-1, 0, 1));
		float      pointSize    = 0.01f;

		public void Initialize()
		{
			// Generate colored points in the shape of a UV sphere! This tool
			// re-uses the Vertex type, but doesn't particularly need the
			// normals, and will replace the UVs. You could pretty easily add
			// lighting to the points using the normals if you wanted to :)
			const int xCount = 24;
			const int yCount = 16;
			Vertex[] points = new Vertex[xCount*yCount];
			for (int y = 0; y < yCount; y++)
			{
				for (int x = 0; x < xCount; x++)
				{
					float u = x / (float)xCount;
					float v = y / (float)yCount;
					int   i = x + y * xCount;

					float theta = u * (float)Math.PI * 2.0f;
					float phi   = (v - 0.5f) * (float)Math.PI;
					float c     = SKMath.Cos(phi);

					points[i].pos = V.XYZ(c * SKMath.Cos(theta), SKMath.Sin(phi), c * SKMath.Sin(theta))*0.2f;
					points[i].col = Color.HSV(u, v, 1).ToLinear();
				}
			}

			// Make a point cloud out of the points!
			cloud      = new PointCloud(pointSize, points);
			cloudScale = 1;
		}

		public void Shutdown()
		{
		}

		public void Update()
		{
			cloud.Draw(cloudPose.ToMatrix(cloudScale));

			UI.WindowBegin("Point Cloud", ref settingsPose);
			{
				if (UI.Button("Load Model"))
				{
					Platform.FilePicker(PickerMode.Open, (file) => {
						Model model = Model.FromFile(file);
						cloud      = new PointCloud(pointSize, model);
						cloudScale = 0.5f / model.Bounds.dimensions.Length;
					}, null, ".gltf", ".glb");
				}
				UI.HSlider("Cloud Scale", ref cloudScale, 0.001f, 2, 0);

				UI.PanelBegin();
				UI.Label("Point Cloud Settings");
				UI.HSeparator();
				UI.Label("Mode:", V.XY(.04f, 0));
				UI.SameLine();
				if (UI.Radio("Fixed", cloud.DistanceIndependantSize)) cloud.DistanceIndependantSize = true;
				UI.SameLine();
				if (UI.Radio("Perspective", !cloud.DistanceIndependantSize)) cloud.DistanceIndependantSize = false;

				UI.Label("Size:", V.XY(.04f, 0));
				UI.SameLine();
				if (UI.HSlider("Point Size", ref pointSize, 0.001f, 0.1f, 0))
					cloud.PointSize = pointSize;
				UI.PanelEnd();
			}
			UI.WindowEnd();

			Text.Add(title, titlePose);
			Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
		}
	}
}
