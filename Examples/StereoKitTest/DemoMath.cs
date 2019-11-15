using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class DemoMath : IDemo
{
    Mesh planeMesh;
    Mesh sphereMesh;
    Mesh cubeMesh;
    Material material;

    public void Update()
    {
        // Plane and Ray
        Vec3  groundPlaneAt = new Vec3(0, -0.25f, 0);
        Plane ground    = new Plane (groundPlaneAt, new Vec3(1,2,0));
        Ray   groundRay = new Ray(Vec3.Zero, new Vec3((float)Math.Cos(Time.Total)*0.5f, -1, (float)Math.Sin(Time.Total)*0.5f).Normalized());

        Lines.Add(groundRay.position, groundRay.position + groundRay.direction * 0.1f, new Color32(255, 0, 0, 255), 2 * Units.mm2m);
        planeMesh.Draw(material, Matrix.TRS(groundPlaneAt, Quat.LookDir(ground.normal), Vec3.One*0.25f));
        if (groundRay.Intersect(ground, out Vec3 groundAt))
            sphereMesh.Draw(material, Matrix.TRS(groundAt, Quat.Identity, Vec3.One * 0.02f), new Color(1, 0, 0, 1));

        // Sphere and Ray
        Sphere sphere    = new Sphere(new Vec3(0,0,-0.5f), 0.25f);
        Vec3   sphereDir = new Vec3((float)Math.Cos(Time.Total), (float)Math.Cos(Time.Total * 3) * 1.5f+0.1f, (float)Math.Sin(Time.Total)).Normalized();
        Ray    sphereRay = new Ray(sphere.center - sphereDir * 0.35f, sphereDir);

        Lines.Add(sphereRay.position, sphereRay.position + sphereRay.direction*0.1f, new Color32(255, 0, 0, 255), 2 * Units.mm2m);
        if (sphereRay.Intersect(sphere, out Vec3 sphereAt))
            sphereMesh.Draw(material, Matrix.TRS(sphereAt, Quat.Identity, Vec3.One * 0.02f), new Color(1,0,0,1));
        sphereMesh.Draw(material, Matrix.TRS(sphere.center, Quat.Identity, Vec3.One*0.25f));

        // Bounds and Ray
        Bounds bounds    = new Bounds(new Vec3(-.5f, 0, -0.5f), Vec3.One * 0.25f);
        Vec3   boundsDir = new Vec3((float)Math.Cos(Time.Total), (float)Math.Cos(Time.Total*3)*1.5f, (float)Math.Sin(Time.Total)).Normalized();
        Ray    boundsRay = new Ray(bounds.center - boundsDir * 0.35f, boundsDir);

        Lines.Add(boundsRay.position, boundsRay.position + boundsRay.direction * 0.1f, new Color32(255, 0, 0, 255), 2 * Units.mm2m);
        if (boundsRay.Intersect(bounds, out Vec3 boundsAt))
            sphereMesh.Draw(material, Matrix.TRS(boundsAt, Quat.Identity, Vec3.One * 0.02f), new Color(1, 0, 0, 1));
        cubeMesh.Draw(material, Matrix.TRS(bounds.center, Quat.Identity, Vec3.One * 0.25f));

        // Bounds and Line
        Bounds boundsLine   = new Bounds(new Vec3(.5f, 0, -0.5f), Vec3.One * 0.25f);
        Vec3   boundsLineP1 = boundsLine.center + new Vec3  ((float)Math.Cos(Time.Total*0.5f), (float)Math.Cos(Time.Total*3), (float)Math.Sin(Time.Total*0.5f)) * 0.35f;
        Vec3   boundsLineP2 = boundsLine.center + new Vec3  ((float)Math.Cos(Time.Total*1),    (float)Math.Cos(Time.Total*6), (float)Math.Sin(Time.Total*1)) * (float)Math.Cos(Time.Total)*0.35f;

        Lines.Add(boundsLineP1, boundsLineP2, new Color32(255,0,0,255), 2*Units.mm2m);
        sphereMesh.Draw(material, Matrix.TRS(boundsLineP1, Quat.Identity, Vec3.One * 0.02f), new Color(1, 0, 0, 1));
        sphereMesh.Draw(material, Matrix.TRS(boundsLineP2, Quat.Identity, Vec3.One * 0.02f), new Color(1, 0, 0, 1));
        cubeMesh  .Draw(material, Matrix.TRS(boundsLine.center, Quat.Identity, Vec3.One * 0.25f),
            boundsLine.Contains(boundsLineP1, boundsLineP2) ? new Color(1,0,0,1) : Color.White);
    }

    public void Initialize() {
        planeMesh  = Mesh.GeneratePlane(Vec2.One,Vec3.Forward, Vec3.Up);
        sphereMesh = Mesh.GenerateSphere(1);
        cubeMesh   = Mesh.GenerateCube(Vec3.One);
        material   = Material.Find(DefaultIds.material);
    }
    public void Shutdown  () { }
}