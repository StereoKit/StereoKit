using StereoKit;
using StereoKit.Framework;
using System;
using System.Collections.Generic;
using System.IO;

namespace StereoKitTest
{
    class Light
    {
        public Pose pose;
        public Vec3 color;
    }
    enum LightMode
    {
        Lights,
        Image,
    }

    class DemoSky : IDemo
    {
        static List<Light> lights     = new List<Light>();
        static Pose        windowPose = new Pose(new Vec3(0,0.1f,-0.3f), Quat.LookDir(-Vec3.Forward));
        static LightMode   mode       = LightMode.Lights;
        static Tex         cubemap    = null;

        Mesh       lightMesh     = Mesh.GenerateSphere(1);
        Material   lightProbeMat = Default.Material;
        Material   lightSrcMat   = new Material(Default.ShaderUnlit);

        public void Initialize() { }
        public void Shutdown() => FilePicker.Hide();
        public void Update()
        {
            UI.WindowBegin("Direction", ref windowPose, new Vec2(20 * Units.cm2m, 0));
            UI.Label("Mode");
            if (UI.Radio("Lights", mode == LightMode.Lights)) mode = LightMode.Lights;
            UI.SameLine();
            if (UI.Radio("Image",  mode == LightMode.Image))  mode = LightMode.Image;

            if (mode == LightMode.Lights)
            { 
                UI.Label("Lights");
                if (UI.Button("Add"))
                {
                    lights.Add(new Light { 
                        pose  = new Pose(Vec3.Up*Units.cm2m*25, Quat.LookDir(-Vec3.Forward)), 
                        color = Vec3.One });
                    UpdateLights();
                }

                UI.SameLine();
                if (UI.Button("Remove") && lights.Count > 1)
                { 
                    lights.RemoveAt(lights.Count-1);
                    UpdateLights();
                }
            }

            if (mode == LightMode.Image)
            {
                UI.Label("Image");
                if (!FilePicker.Active && UI.Button("Open"))
                    ShowPicker();
            }

            UI.WindowEnd();

            lightMesh.Draw(lightProbeMat, Matrix.TS(Vec3.Zero, 0.04f));

            if (mode == LightMode.Lights)
            { 
                bool needsUpdate = false;
                for (int i = 0; i < lights.Count; i++) {
                    needsUpdate = LightHandle(i) || needsUpdate;
                }
                if (needsUpdate)
                    UpdateLights();
            }
        }

        bool LightHandle(int i)
        {
            UI.PushId("window"+i);
            bool dirty = UI.AffordanceBegin("Color", ref lights[i].pose, new Bounds(Vec3.One * 3 * Units.cm2m));
            UI.LayoutArea(new Vec3(6,-3,0)*Units.cm2m, new Vec2(10, 0) * Units.cm2m);
            if (lights[i].pose.position.Magnitude > 0.5f)
                lights[i].pose.position = lights[i].pose.position.Normalized() * 0.5f;

            lightMesh.Draw(lightSrcMat, Matrix.TS(Vec3.Zero, 3*Units.cm2m), Color.HSV(lights[i].color));

            dirty = UI.HSlider("H", ref lights[i].color.x, 0, 1, 0, 10 * Units.cm2m) || dirty;
            dirty = UI.HSlider("S", ref lights[i].color.y, 0, 1, 0, 10 * Units.cm2m) || dirty;
            dirty = UI.HSlider("V", ref lights[i].color.z, 0, 1, 0, 10 * Units.cm2m) || dirty;
            
            UI.AffordanceEnd();
            Lines.Add(
                lights[i].pose.position, Vec3.Zero, 
                Color.HSV(lights[i].color) * LightIntensity(lights[i].pose.position) * 0.5f, 
                Units.mm2m);

            UI.PopId();
            return dirty;
        }

        void ShowPicker()
        {
            FilePicker.Show(
                Path.GetFullPath(StereoKitApp.settings.assetsFolder),
                LoadSkyImage,
                new FileFilter("HDR", "*.hdr"));
        }

        void LoadSkyImage(string file)
        {
            cubemap = Tex.FromCubemapEquirectangular(file, out SphericalHarmonics lighting);

            Renderer.SkyTex   = cubemap;
            Renderer.SkyLight = lighting;
        }

        void UpdateLights()
        {
            SphericalHarmonics lighting = SphericalHarmonics.FromLights(lights
                .ConvertAll(a => new SHLight{ 
                    directionTo = a.pose.position.Normalized(),
                    color       = Color.HSV(a.color) * LightIntensity(a.pose.position) })
                .ToArray());

            Renderer.SkyTex   = Tex.GenCubemap(lighting);
            Renderer.SkyLight = lighting;
        }

        float LightIntensity(Vec3 pos)
        {
            return Math.Max(0, 2 - pos.Magnitude * 4);
        }
    }
}
