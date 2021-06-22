using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class DemoLineRender : ITest
{
	struct Body
	{
		public Vec3    position;
		public Vec3    velocity;
		public Vec3    force;
		public float   mass;
		public Color   color;
		public List<LinePoint> trail;
	}

	int    trailId = 0;
	Vec3   center = new Vec3(0.4f,0,-0.4f);
	Body[] bodies;

	public void Initialize()
	{
		bodies   = new Body[5];
		Random r = new Random();
		for (int i = 0; i < bodies.Length; i++)
		{
			bodies[i].position = center + V.XYZ((float)r.NextDouble()-0.5f, (float)r.NextDouble() - 0.5f, (float)r.NextDouble() - 0.5f);
			bodies[i].velocity = V.XYZ((float)r.NextDouble()-0.5f, (float)r.NextDouble()-0.5f, (float)r.NextDouble()-0.5f);
			bodies[i].mass     = (float)r.NextDouble() + 0.5f;
			bodies[i].color    = Color.HSV((float)r.NextDouble(), 0.8f, 0.8f);
			bodies[i].trail    = new List<LinePoint>();
		}
	}

	public void Shutdown()
	{
	}

	public void Update()
	{
		Default.MeshSphere.Draw(Default.MaterialUnlit, Matrix.TS(center, 0.06f), Color.HSV(0.16f, 0.4f, 0.9f));
		for (int i = 0; i < bodies.Length; i++) { 
			Vec3 force = (bodies[i].position-center) * -200;

			// Make hands have mass
			for (int h = 0; h < (int)Handed.Max; h++)
			{
				Hand hand = Input.Hand((Handed)h);
				if (!hand.IsTracked) continue;

				Vec3 dir = hand.palm.position - bodies[i].position;
				force += dir.Normalized * 300 * hand.gripActivation;
			}

			for (int t = 0; t < bodies.Length; t++) { 
				if (i == t) continue;

				Vec3  dir       = bodies[t].position - bodies[i].position;
				Vec3  currForce = dir.Normalized * ((dir.MagnitudeSq * 50) / bodies[i].mass);
				force += currForce;
			}

			if (force.MagnitudeSq > 1)
				force.Normalize();
			bodies[i].force = force;
		}
		for (int i = 0; i < bodies.Length; i++)
		{
			bodies[i].velocity += bodies[i].force * (Time.Elapsedf/bodies[i].mass);
			if (bodies[i].velocity.MagnitudeSq > 1)
				bodies[i].velocity.Normalize();
			bodies[i].position += bodies[i].velocity * Time.Elapsedf;

			Color col = bodies[i].color * Math.Min(0.5f, (bodies[i].position-center).MagnitudeSq/0.5f);
			col.a = 1;
			if (trailId == i)
			{
				bodies[i].trail.Add(new LinePoint(bodies[i].position, col, 0.02f));
				if (bodies[i].trail.Count > 1000) bodies[i].trail.RemoveAt(0);
			} else if (bodies[i].trail.Count > 0)
			{
				bodies[i].trail[bodies[i].trail.Count-1] = new LinePoint(bodies[i].position, col, 0.02f);
			}
			Default.MeshSphere.Draw(Default.Material, Matrix.TS(bodies[i].position, 0.03f * bodies[i].mass), col);
			Lines.Add(bodies[i].trail.ToArray());
		}
		trailId = (trailId + 1) % bodies.Length;
	}
}