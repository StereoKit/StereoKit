using System;
using System.Collections.Generic;
using System.Security.Cryptography;

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
        public string         layerName;
        public HandMenuItem[] items;
        public float          startAngle;

        public HandRadialLayer(string name, params HandMenuItem[] items):
            this(name, 0, items) { }
        public HandRadialLayer(string name, float startAngle, params HandMenuItem[] items)
        {
            this.layerName  = name;
            this.startAngle = startAngle;
            this.items      = items;
        }
    }

    public class HandMenuRadial : IStepper
    {
        bool showHandSubmenu = false;
        Pose subMenuPose;
        Pose destPose;

        HandRadialLayer[] layers;
        Stack<int>        navStack = new Stack<int>();
        int               activeLayer;
        LinePoint[]       circle;
        LinePoint[]       innerCircle;
        float             scale = minScale;

        const float minDist = 0.03f;
        const float midDist = 0.065f;
        const float maxDist = 0.1f;
        const float minScale = 0.1f;

        public HandMenuRadial(params HandRadialLayer[] menuLayers)
        {
            layers = menuLayers;
            activeLayer = 0;

            circle      = new LinePoint[48];
            innerCircle = new LinePoint[circle.Length];
            float step = 360.0f / (circle.Length-1);
            for (int i = 0; i < circle.Length; i++)
            {
                Vec3 dir = Vec3.AngleXY(i * step);
                circle[i].pt        = dir*maxDist;
                circle[i].color     = Color.White * 0.5f;
                circle[i].thickness = 2 * Units.mm2m;
                innerCircle[i].pt        = dir * minDist;
                innerCircle[i].color     = Color.White * 0.5f;
                innerCircle[i].thickness = 2 * Units.mm2m;
            }
        }

        public void Close()
        {
            scale = minScale;
            activeLayer = 0;
            showHandSubmenu = false;
            navStack.Clear();
        }

        public bool Enabled => true;

        public bool Initialize() => true;

        public void Shutdown() {}

        static bool HandFacingHead(Hand hand)
        {
            if (!hand.IsTracked)
                return false;

            Vec3 palmDirection   = (hand.palm.Forward).Normalized();
            Vec3 directionToHead = (Input.Head.position - hand.palm.position).Normalized();

            return Vec3.Dot(palmDirection, directionToHead) > 0.5f;
        }

        public void Step()
        {
            Hand hand   = Input.Hand(Handed.Right);
            bool facing = HandFacingHead(hand);

            if (facing && !showHandSubmenu)
            {
                if (hand.IsJustGripped)
                {
                    destPose = hand[FingerId.Index, JointId.Tip].Pose;
                    destPose.orientation = Quat.LookAt(subMenuPose.position, Input.Head.position);
                    showHandSubmenu = true;
                } 
                else
                {
                    float d1 = Vec3.Distance(
                        hand[FingerId.Little, JointId.Tip].position,
                        hand[FingerId.Little, JointId.Metacarpal].position);
                    float d2 = Vec3.Distance(
                        hand[FingerId.Little, JointId.Tip].position,
                        hand[FingerId.Little, JointId.Metacarpal].position);
                    const float previewDist = 14 * Units.cm2m;
                    scale = minScale + ((previewDist-(d1+d2)) / previewDist) * 0.2f;

                    subMenuPose.position = 
                       (hand[FingerId.Little, JointId.Tip].position +
                        hand[FingerId.Middle, JointId.Tip].position +
                        hand[FingerId.Little, JointId.Metacarpal].position +
                        hand[FingerId.Middle, JointId.Metacarpal].position) * 0.25f;
                    subMenuPose.orientation = Quat.LookAt(subMenuPose.position, Input.Head.position);
                    subMenuPose.position += subMenuPose.Forward*Units.cm2m;
                }
            }

            if (showHandSubmenu)
            {
                float time = (Time.Elapsedf * 24);
                subMenuPose.position    = Vec3.Lerp (subMenuPose.position,    destPose.position,    time);
                subMenuPose.orientation = Quat.Slerp(subMenuPose.orientation, destPose.orientation, time);

                scale = scale + (1-scale) * time;
                HandRadialLayer layer = layers[activeLayer];
                int   count    = layer.items.Length;
                float step     = 360 / count;
                float halfStep = step/2;

                Hierarchy.Push(subMenuPose.ToMatrix(scale));
                Lines.Add(circle);
                Lines.Add(innerCircle);

                Vec3  fingertip = Hierarchy.ToLocal(hand[FingerId.Index, JointId.Tip].position);
                float magSq    = fingertip.MagnitudeSq;
                float angle    = (float)Math.Atan2(fingertip.y, fingertip.x) * Units.rad2deg - layer.startAngle;
                bool  onPlane  = fingertip.z > -0.02f && fingertip.z < 0.02f;
                bool  active   = onPlane && magSq > minDist * minDist;
                bool  selected = onPlane && magSq > midDist * midDist;
                bool  unfocus  = magSq > maxDist*maxDist;

                while (angle < 0)
                    angle += 360;
                int angleId = (int)(angle / step);

                for (int i = 0; i < count; i++)
                {
                    float currAngle = i*step + layer.startAngle;
                    bool highlightText = active && angleId == i;
                    bool highlightLine = highlightText || (active && (angleId+1)%6 == i);
                    Vec3 dir = Vec3.AngleXY(currAngle);
                    Lines.Add(dir * minDist, dir * maxDist, highlightLine ? Color.White : Color.White*0.5f, highlightLine?0.002f:0.001f);
                    Text.Add(layer.items[i].name, Matrix.TRS(Vec3.AngleXY(currAngle + halfStep)*midDist, Quat.FromAngles(0, 0, currAngle + halfStep - 90), highlightText?1.2f:1), TextAlign.XCenter | TextAlign.YBottom);
                }
                Lines.Add(Vec3.Zero, new Vec3(fingertip.x, fingertip.y, 0), Color.White * 0.5f, 0.001f);

                Hierarchy.Pop();

                if (scale > 0.95f) { 
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
                            destPose.position = plane.Closest(hand[FingerId.Index, JointId.Tip].position);
                        }

                        item.callback?.Invoke();
                    }
                }
            }
            else if (facing)
            {
                Hierarchy.Push(subMenuPose.ToMatrix(scale));
                Lines.Add(circle);
                Hierarchy.Pop();
            }
        }
    }
}
