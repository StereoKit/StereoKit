using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class DemoMath : IDemo
{
    Mesh sphereMesh;
    Material material;

    public void Update()
    {
        Plane ground    = new Plane (new Vec3(0,-1,0), Vec3.Up);
        Ray   groundRay = new Ray(Vec3.Zero, new Vec3((float)Math.Cos(Time.Total), 1, (float)Math.Sin(Time.Total))*0.25f);
        if (groundRay.Intersect(ground, out Vec3 groundAt))
            Renderer.Add(sphereMesh, material, Matrix.TRS(groundAt, Quat.Identity, Vec3.One * 0.02f), new Color(1, 0, 0, 1));

        Sphere sphere    = new Sphere(new Vec3(0,0,-0.5f), 0.25f);
        Ray    sphereRay = new Ray(Vec3.Right*(float)Math.Sin(Time.Total)*0.2f, Vec3.Forward);

        if (sphereRay.Intersect(sphere,out Vec3 sphereAt))
            Renderer.Add(sphereMesh, material, Matrix.TRS(sphereAt, Quat.Identity, Vec3.One * 0.02f), new Color(1,0,0,1));
        Renderer.Add(sphereMesh, material, Matrix.TRS(sphere.center, Quat.Identity, Vec3.One*0.25f));
    }

    public void Initialize() {
        sphereMesh = Mesh.GenerateSphere(1);
        material   = Material.Find(DefaultIds.material);
    }
    public void Shutdown  () { }
}