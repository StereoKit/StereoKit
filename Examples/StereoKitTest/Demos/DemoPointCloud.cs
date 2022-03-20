using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StereoKitTest.Demos
{
	public class PointCloud
	{
		Material mat;
		Vertex[] verts;
		Mesh     mesh;

		bool  distanceIndependant;
		float pointSize;

		public bool  DistanceIndependantSize { get => distanceIndependant; set { mat["screen_size"] = value?1:0; distanceIndependant = value; } }
		public float PointSize               { get => pointSize;           set { mat["point_size" ] = value;     pointSize           = value; } }

		public PointCloud(float pointSizeMeters = 0.01f)
		{
			mat = new Material(Shader.FromFile("point_cloud.hlsl"));
			PointSize = pointSizeMeters;
		}
		public PointCloud(float pointSizeMeters, Mesh fromMesh)
			: this (pointSizeMeters) => SetPoints(fromMesh);
		public PointCloud(float pointSizeMeters, Vertex[] fromVerts)
			: this(pointSizeMeters) => SetPoints(fromVerts);

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
				verts[idx  ] = new Vertex(p.pos, Vec3.Up, V.XY(-.5f, .5f), p.col);
				verts[idx+1] = new Vertex(p.pos, Vec3.Up, V.XY( .5f, .5f), p.col);
				verts[idx+2] = new Vertex(p.pos, Vec3.Up, V.XY( .5f,-.5f), p.col);
				verts[idx+3] = new Vertex(p.pos, Vec3.Up, V.XY(-.5f,-.5f), p.col);
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
	}

	class DemoPointCloud : ITest
	{
		PointCloud cloud;
		public void Initialize()
		{
			Model model = Model.FromFile("DamagedHelmet.gltf");
			cloud = new PointCloud(0.01f, model.Visuals[0].Mesh);
		}

		public void Shutdown()
		{
		}

		public void Update()
		{
			cloud.Draw(Matrix.Identity);

			if (Input.Key(Key.O).IsJustActive()) cloud.DistanceIndependantSize = true;
			if (Input.Key(Key.P).IsJustActive()) cloud.DistanceIndependantSize = false;
			if (Input.Key(Key.BracketOpen ).IsJustActive()) cloud.PointSize -= 0.0025f;
			if (Input.Key(Key.BracketClose).IsJustActive()) cloud.PointSize += 0.0025f;
		}
	}
}
