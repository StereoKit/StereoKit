using System;
using System.Collections.Generic;

namespace StereoKit.Framework
{
    public enum HandMenuAction
    {
        Layer,
        Back,
        Close
    }
    public class HandMenuItem
    {
        public string name;
        public Sprite image;

        public HandMenuAction action;
        public Action callback;
        public string layerName;

        public HandMenuItem(string name, Sprite image, Action callback, string layerName) 
            : this(name, image, callback, HandMenuAction.Layer)
        {
            this.layerName = layerName;
        }
        public HandMenuItem(string name, Sprite image, Action callback, HandMenuAction action = HandMenuAction.Close)
        {
            this.name     = name;
            this.image    = image;
            this.action   = action;
            this.callback = callback;
        }
    }
    public class HandRadialLayer
    {
        public string layerName;
        public HandMenuItem[] items;

        public HandRadialLayer(string name, params HandMenuItem[] items)
        {
            this.layerName = name;
            this.items = items;
        }
    }

    public class HandMenuRadial : IStepper
    {
        bool showHandSubmenu = false;
        Pose subMenuPose;

        HandRadialLayer[] layers;
        Stack<int>        navStack = new Stack<int>();
        int               activeLayer;

        public HandMenuRadial(params HandRadialLayer[] menuLayers)
        {
            layers = menuLayers;
            activeLayer = 0;
        }

        public void Close()
        {
            activeLayer = 0;
            showHandSubmenu = false;
            navStack.Clear();
        }

        public bool Enabled => true;

        public bool Initialize() => true;

        public void Shutdown() {}

        public void Step()
        {
            Hand hand = Input.Hand(Handed.Right);
            
            if (!showHandSubmenu && hand.IsJustGripped)
            {
                subMenuPose = hand[FingerId.Index, JointId.Tip].Pose;
                subMenuPose.orientation = Quat.LookAt(subMenuPose.position, Input.Head.position);
                showHandSubmenu = true;
            }

            if (showHandSubmenu)
            {
                HandRadialLayer layer = layers[activeLayer];

                const float minDist = 0.03f;
                const float midDist = 0.065f;
                const float maxDist = 0.1f;
                int   count    = layer.items.Length;
                float step     = 360 / count;
                float halfStep = step/2;

                Hierarchy.Push(subMenuPose.ToMatrix());
                Vec3  fingertip = Hierarchy.ToLocal(hand[FingerId.Index, JointId.Tip].position);
                float magSq    = fingertip.MagnitudeSq;
                float angle    = (float)Math.Atan2(fingertip.y, fingertip.x) * Units.rad2deg;
                bool  onPlane  = fingertip.z > -0.02f && fingertip.z < 0.02f;
                bool  active   = onPlane && magSq > minDist * minDist;
                bool  selected = onPlane && magSq > midDist * midDist;
                bool  unfocus  = magSq > maxDist*maxDist;

                if (angle < 0)
                    angle += 360;
                int angleId = (int)(angle / step);

                for (int i = 0; i < count; i++)
                {
                    bool highlightText = active && angleId == i;
                    bool highlightLine = highlightText || (active && (angleId+1)%6 == i);
                    Vec3 dir = Vec3.AngleXY(i * step);
                    Lines.Add(dir * minDist, dir * maxDist, highlightLine ? Color.White : Color.White*0.5f, highlightLine?0.002f:0.001f);
                    Text.Add(layer.items[i].name, Matrix.TRS(Vec3.AngleXY(i * step + halfStep)*midDist, Quat.FromAngles(0, 0, i * step + halfStep - 90), highlightText?1.2f:1), TextAlign.XCenter | TextAlign.YBottom);
                }
                Lines.Add(Vec3.Zero, new Vec3(fingertip.x, fingertip.y, 0), Color.White * 0.5f, 0.001f);

                Hierarchy.Pop();

                if (unfocus)
                    Close();

                if (selected)
                {
                    HandMenuItem item = layer.items[angleId];
                    if (item.action == HandMenuAction.Close)
                    { 
                        Close();
                    }
                    else if (item.action == HandMenuAction.Layer) 
                    {
                        navStack.Push(activeLayer);
                        activeLayer = Array.FindIndex(layers, l=>l.layerName == item.layerName);
                        if (activeLayer == -1)
                            Log.Err($"Couldn't find hand menu layer named {item.layerName}!");
                    } 
                    else if (item.action == HandMenuAction.Back) 
                    {
                        if (navStack.Count > 0)
                            activeLayer = navStack.Pop();
                    }

                    if (item.action == HandMenuAction.Back || item.action == HandMenuAction.Layer)
                    {
                        Plane plane = new Plane(subMenuPose.position, subMenuPose.Forward);
                        subMenuPose.position = plane.Closest(hand[FingerId.Index, JointId.Tip].position);
                    }

                    item.callback?.Invoke();
                }
            }
        }
    }
}
