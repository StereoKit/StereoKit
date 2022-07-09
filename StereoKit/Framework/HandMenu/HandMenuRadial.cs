using System;
using System.Collections.Generic;

namespace StereoKit.Framework
{
	/// <summary>This enum specifies how HandMenuItems should behave
	/// when selected! This is in addition to the HandMenuItem's 
	/// callback function.</summary>
	public enum HandMenuAction
	{
		/// <summary>Move to another menu layer.</summary>
		Layer,
		/// <summary>Go back to the previous layer.</summary>
		Back,
		/// <summary>Close the hand menu entirely! We're finished here.</summary>
		Close
	}

	/// <summary>A menu that shows up in circle around the user's 
	/// hand! Selecting an item can perform an action, or even spawn
	/// a sub-layer of menu items. This is an easy way to store 
	/// actions out of the way, yet remain easily accessible to the
	/// user.
	/// 
	/// The user faces their palm towards their head, and then makes
	/// a grip motion to spawn the menu. The user can then perform actions
	/// by making fast, direction based motions that are easy to build 
	/// muscle memory for.</summary>
	public class HandMenuRadial : IStepper
	{
		#region Fields
		/// <summary>When using the Simulator, this key will activate the menu
		/// on the current hand, regardless of which direction it is facing.
		/// </summary>
		public static Key simulatorKey = Key.Backtick;

		const float minDist  = 0.03f;
		const float midDist  = 0.065f;
		const float maxDist  = 0.1f;
		const float minScale = 0.05f;
		const float sliceGap = 0.002f;
		const float outOfViewAngle = 0.866f; // 0.866 is 30 degrees with cos baked in: Math.Cos(30 * Units.deg2rad);
		const float activationAngle = 0.99f; // 0.99 is 8 degrees with cos baked in: Math.Cos(8 * Units.deg2rad);

		Pose menuPose;
		Pose destPose;

		HandRadialLayer        root        = null;
		HandRadialLayer        activeLayer = null;
		Stack<HandRadialLayer> navStack    = new Stack<HandRadialLayer>();
		Handed                 activeHand  = Handed.Max;
		float                  activation  = 0;
		float                  menuScale   = 0;
		float                  angleOffset = 0;

		Mesh background;
		Mesh backgroundEdge;
		Mesh activationButton;
		Mesh activationRing;
		Mesh childIndicator;
		#endregion

		#region Public Methods

		/// <summary>Creates a hand menu from the provided array of menu 
		/// layers! HandMenuRadial is an IStepper, so proper usage is to
		/// add it to the Stepper list via `StereoKitApp.AddStepper`. If no
		/// layers are provided to this constructor, a default root layer will
		/// be automatically added.</summary>
		/// <param name="menuLayers">Starting layer is always the first one
		/// in the list! Layer names in the menu items refer to layer names
		/// in this list.</param>
		public HandMenuRadial(params HandRadialLayer[] menuLayers)
		{
			root = menuLayers != null && menuLayers.Length > 0
				? menuLayers[0]
				: new HandRadialLayer( "Root",
					new HandMenuItem( "Cancel", null, null, HandMenuAction.Close ) );

			foreach (HandRadialLayer layer in menuLayers)
			{
				foreach (HandMenuItem item in layer.items)
				{
					if (!string.IsNullOrEmpty(item.layerName))
					{
						HandRadialLayer link = Array.Find(menuLayers, m => m.layerName == item.layerName);
						layer.childLayers.Add(link);
						link.parent    = layer;
						link.layerItem = item;
					}
				}
			}

			float activationBtnRadius = 1 * U.cm;
			activationButton = GenerateActivationButton(activationBtnRadius);
			GenerateSliceMesh(360, activationBtnRadius, activationBtnRadius + 0.005f, 0, ref activationRing);
			childIndicator = GenerateChildIndicator(maxDist - 0.008f, 0.004f);
		}

		/// <summary>This returns the root menu layer, this is always the first
		/// layer added to the list of layers. There should always be a root
		/// layer, and if the HandMenuRadial is created without any layers, a
		/// root layer will be added automatically.</summary>
		public HandRadialLayer RootLayer => root;

		/// <summary>Force the hand menu to show at a specific location.
		/// This will close the hand menu if it was already open, and resets
		/// it to the root menu layer. Also plays an opening sound.</summary>
		/// <param name="at">A world space position for the hand menu.</param>
		public void Show(Vec3 at, Handed hand)
		{
			if (activeHand != Handed.Max)
				Close();
			Default.SoundClick.Play(at);
			destPose.position    = at;
			destPose.orientation = Quat.LookAt(at, Input.Head.position);
			activeLayer = root;
			activeHand  = hand;

			GenerateSliceMesh(360 / activeLayer.items.Length, minDist, maxDist, sliceGap, ref background);
			GenerateSliceMesh(360 / activeLayer.items.Length, maxDist, maxDist + 0.005f, sliceGap, ref backgroundEdge);
		}

		/// <summary>Closes the menu if it's open! Plays a closing sound.</summary>
		public void Close()
		{
			if (activeHand == Handed.Max)
				return;
			Default.SoundUnclick.Play(menuPose.position);
			menuScale   = minScale;
			activeHand  = Handed.Max;
			angleOffset = 0;
			navStack.Clear();
		}

		/// <summary>Part of IStepper, you shouldn't be calling this yourself.</summary>
		public void Step()
		{
			if (activeHand == Handed.Max)
			{
				for (int i = 0; i < (int)Handed.Max; i++)
					StepMenuIndicator((Handed)i);
			}

			if (activeHand != Handed.Max)
				StepMenu(Input.Hand(activeHand));
		}

		/// <summary>HandMenuRadial is always Enabled.</summary>
		public bool Enabled => true;
		/// <summary>Part of IStepper, you shouldn't be calling this yourself.</summary>
		/// <returns>Always returns true.</returns>
		public bool Initialize() => true;
		/// <summary>Part of IStepper, you shouldn't be calling this yourself.</summary>
		public void Shutdown() { }

		#endregion

		#region Private Methods

		void StepMenuIndicator(Handed handed)
		{
			Hand hand = Input.Hand(handed);
			if (!hand.IsTracked) return;

			// In the flatscreen simulator, display this menu on backtick
			if (Backend.XRType == BackendXRType.Simulator)
			{
				if (Input.Key(simulatorKey).IsJustActive())
				{
					menuPose = hand.palm;
					Show(hand[FingerId.Index, JointId.Tip].position, handed);
				}
				return;
			}

			// If the hand is not in the field of view, we want to ignore it to
			// help prevent mis-presses.
			Vec3 headFwd = Input.Head.Forward;
			Vec3 handDir = (hand.palm.position - Input.Head.position).Normalized;
			bool inView  = Vec3.Dot(headFwd, handDir) > outOfViewAngle;

			if (!inView) return;

			// Get information about how directly the hand is facing towards
			// the user's face.
			Vec3  palmDirection   = hand.palm.Forward;
			Vec3  directionToHead = -handDir;
			float facing          = Vec3.Dot(palmDirection, directionToHead);

			if (facing < 0) return;

			// Draw the menu button!
			Color colorPrimary = UI.GetThemeColor(UIColor.Primary   ).ToLinear();
			Color colorCommon  = UI.GetThemeColor(UIColor.Background).ToLinear();
			menuPose = hand.palm;
			activationRing.Draw(Material.UI, menuPose.ToMatrix(), colorPrimary);
			menuPose.position += (1- hand.gripActivation) * menuPose.Forward*U.cm*2;
			activationButton.Draw(Material.UI, menuPose.ToMatrix(), Color.Lerp(colorCommon, colorPrimary, Math.Max(0,Math.Min(1,(facing-(activationAngle-0.01f))/0.01f))));

			if (facing < activationAngle) return;

			// And if the user grips, show the menu!
			if (hand.IsJustGripped)
				Show(hand[FingerId.Index, JointId.Tip].position, handed);
		}

		void StepMenu(Hand hand)
		{
			// Animate the menu a bit
			float time = Math.Min(1, Time.Elapsedf * 24);
			menuPose.position    = Vec3  .Lerp (menuPose.position,    destPose.position,    time);
			menuPose.orientation = Quat  .Slerp(menuPose.orientation, destPose.orientation, time);
			activation           = SKMath.Lerp (activation,           1,                    time);
			menuScale            = SKMath.Lerp (menuScale,            1,                    time);

			// Pre-calculate some circle traversal values
			HandRadialLayer layer = activeLayer;
			int   count    = layer.items.Length;
			float step     = 360 / count;
			float halfStep = step/2;

			// Push the Menu's pose onto the stack, so we can draw, and work
			// in local space.
			Hierarchy.Push(menuPose.ToMatrix(menuScale));

			// Calculate the status of the menu!
			Vec3  tipWorld  = hand[FingerId.Index, JointId.Tip].position;
			Vec3  tipLocal  = destPose.ToMatrix().Inverse.Transform(tipWorld);
			float magSq     = tipLocal.MagnitudeSq;
			bool  onMenu    = tipLocal.z > -0.02f && tipLocal.z < 0.02f;
			bool  focused   = onMenu && magSq > minDist * minDist;
			bool  selected  = onMenu && magSq > midDist * midDist;
			bool  cancel    = magSq > maxDist*maxDist;
			float fitRadius = midDist * Math.Min(90, step) * Units.deg2rad;

			// Find where our finger is pointing to, and draw that
			float fingerAngle = (float)Math.Atan2(tipLocal.y, tipLocal.x) * Units.rad2deg - (layer.startAngle + angleOffset);
			while (fingerAngle < 0) fingerAngle += 360;
			int angleId = (int)(fingerAngle / step);
			Lines.Add(V.XYZ(0,0,-0.008f), V.XYZ(tipLocal.x, tipLocal.y, -0.008f), Color.White, 0.006f);

			// Now draw each of the menu items!
			Color colorPrimary = UI.GetThemeColor(UIColor.Primary   ).ToLinear();
			Color colorCommon  = UI.GetThemeColor(UIColor.Background).ToLinear();
			for (int i = 0; i < count; i++)
			{
				float  currAngle = i*step + layer.startAngle + angleOffset;
				bool   highlight = focused && angleId == i && activation >= 0.99f;
				float  depth     = highlight ? -0.005f : 0.0f;
				Vec3   at        = Vec3.AngleXY(currAngle + halfStep) * midDist;
				at.z = depth;

				Matrix r = Matrix.TR(0,0,depth, Quat.FromAngles( 0, 0, currAngle ));
				background    .Draw(Material.UI, r, colorCommon  * (highlight ? 2.0f:1.0f));
				backgroundEdge.Draw(Material.UI, r, colorPrimary * (highlight ? 2.0f:1.0f));
				if (layer.items[i].action == HandMenuAction.Layer || layer.items[i].action == HandMenuAction.Back )
					childIndicator.Draw(Material.UI, Matrix.TR(0, 0, depth, Quat.FromAngles(0, 0, currAngle + halfStep)));
				layer.items[i].Draw(at, fitRadius, currAngle + halfStep, highlight);
			}

			// Done with local work
			Hierarchy.Pop();

			// Execute any actions that were discovered

			// But not if we're still in the process of animating, interaction
			// values could be pretty incorrect when we're still lerping
			// around.
			if (activation < 0.99f) return;
			if (selected)           SelectItem(layer.items[angleId], tipWorld, (angleId + 0.5f) * step);
			if (cancel)             Close();
		}

		void SelectLayer(string name)
		{
			Default.SoundClick.Play(menuPose.position);
			navStack.Push(activeLayer);
			activeLayer = activeLayer.FindChild(name);
			if (activeLayer == null)
				Log.Err($"Couldn't find hand menu layer named {name}!");

			GenerateSliceMesh(360 / activeLayer.items.Length, minDist, maxDist, sliceGap, ref background);
			GenerateSliceMesh(360 / activeLayer.items.Length, maxDist, maxDist + 0.005f, sliceGap, ref backgroundEdge);
		}

		void Back()
		{
			Default.SoundUnclick.Play(menuPose.position);
			if (navStack.Count > 0)
				activeLayer = navStack.Pop();

			GenerateSliceMesh(360 / activeLayer.items.Length, minDist, maxDist, sliceGap, ref background);
			GenerateSliceMesh(360 / activeLayer.items.Length, maxDist, maxDist + 0.005f, sliceGap, ref backgroundEdge);
		}

		void SelectItem(HandMenuItem item, Vec3 at, float fromAngle)
		{
			if      (item.action == HandMenuAction.Close) Close      ();
			else if (item.action == HandMenuAction.Layer) SelectLayer(item.layerName);
			else if (item.action == HandMenuAction.Back ) Back       ();

			if (item.action == HandMenuAction.Back || item.action == HandMenuAction.Layer)
				Reposition(at, fromAngle);

			item.callback?.Invoke();
		}

		void Reposition(Vec3 at, float fromAngle)
		{
			Plane plane = new Plane(menuPose.position, menuPose.Forward);
			destPose.position = plane.Closest(at);
			activation = 0;

			if (activeLayer.backAngle != 0)
			{
				angleOffset = (fromAngle - activeLayer.backAngle) + 180;
				while(angleOffset < 0  ) angleOffset += 360;
				while(angleOffset > 360) angleOffset -= 360;
			} else angleOffset = 0;
		}

		static void GenerateSliceMesh(float angle, float minDist, float maxDist, float gap, ref Mesh mesh)
		{
			int count = (int)(angle * 0.25f);

			float innerStartAngle = (gap / (minDist * Units.deg2rad));
			float innerAngle      = angle - innerStartAngle * 2;
			float innerStep       = innerAngle / (count - 1);

			float outerStartAngle = (gap / (maxDist * Units.deg2rad));
			float outerAngle      = angle - outerStartAngle * 2;
			float outerStep       = outerAngle / (count - 1);

			Vertex[] verts = new Vertex[count * 2];
			uint  [] inds  = new uint[(count-1) * 6];

			for (uint i = 0; i < count; i++)
			{
				Vec3 innerDir = Vec3.AngleXY(innerStartAngle + i * innerStep, 0.005f);
				Vec3 outerDir = Vec3.AngleXY(outerStartAngle + i * outerStep, 0.005f);
				verts[i * 2    ] = new Vertex(innerDir * minDist, Vec3.Forward);
				verts[i * 2 + 1] = new Vertex(outerDir * maxDist, Vec3.Forward);

				if (i == count - 1) continue;
				inds[i * 6+2] = i * 2;
				inds[i * 6+1] = i * 2 + 1;
				inds[i * 6+0] = (i+1) * 2 + 1;

				inds[i * 6+5] = i * 2;
				inds[i * 6+4] = (i+1) * 2 + 1;
				inds[i * 6+3] = (i+1) * 2;
			}

			if (mesh == null) mesh = new Mesh();
			mesh.SetVerts(verts);
			mesh.SetInds (inds);
		}

		static Mesh GenerateActivationButton(float radius)
		{
			int      spokes = 36;
			Vertex[] verts  = new Vertex[spokes + 12];
			uint  [] inds   = new uint[(spokes-2)*3 + 6*3];

			// A circle of vertices
			for (uint i = 0; i < spokes; i++)
				verts[i] = new Vertex(Vec3.AngleXY(i * (360.0f/(spokes))) * radius, Vec3.Forward);

			// No vertex in the center, so we're adding a strip of triangles
			// across the circle instead
			for (uint i = 0; i < spokes-2; i++)
			{
				uint half = i / 2;
				if (i%2 == 0) // even
				{
					inds[i*3+2] = (uint)((spokes - half) % spokes);
					inds[i*3+1] = half + 1;
					inds[i*3  ] = (uint)((spokes - 1) - half);
				}
				else // odd
				{
					inds[i*3  ] = half + 1;
					inds[i*3+1] = (uint)(spokes - (half+1));
					inds[i*3+2] = half + 2;
				}
			}

			// Add white bars to indicate a hamburger menu
			float w = radius / 3;
			float h = radius / 16;
			float z = -0.003f;
			int curr = (spokes - 2) * 3;
			for (int i = 0; i < 3; i++)
			{
				float y = -radius / 3 + i * radius / 3;

				uint a = (uint)(spokes + i * 4);
				uint b = (uint)(spokes + i * 4 + 1);
				uint c = (uint)(spokes + i * 4 + 2);
				uint d = (uint)(spokes + i * 4 + 3);
				verts[a] = new Vertex(new Vec3(-w, y - h, z), Vec3.Forward, Vec2.Zero, new Color32(255,255,255,0));
				verts[b] = new Vertex(new Vec3( w, y - h, z), Vec3.Forward, Vec2.Zero, new Color32(255,255,255,0));
				verts[c] = new Vertex(new Vec3( w, y + h, z), Vec3.Forward, Vec2.Zero, new Color32(255,255,255,0));
				verts[d] = new Vertex(new Vec3(-w, y + h, z), Vec3.Forward, Vec2.Zero, new Color32(255,255,255,0));

				inds[curr++] = c;
				inds[curr++] = b;
				inds[curr++] = a;

				inds[curr++] = d;
				inds[curr++] = c;
				inds[curr++] = a;
			}

			Mesh m = new Mesh();
			m.SetInds (inds);
			m.SetVerts(verts);
			return m;
		}

		static Mesh GenerateChildIndicator(float distance, float radius)
		{
			Vertex[] verts = new Vertex[3];
			uint  [] inds  = new uint  [3];

			verts[0] = new Vertex(V.XYZ(distance,           radius*2, 0), Vec3.Forward);
			verts[1] = new Vertex(V.XYZ(distance + radius,  0,        0), Vec3.Forward);
			verts[2] = new Vertex(V.XYZ(distance,          -radius*2, 0), Vec3.Forward);

			inds[0] = 0;
			inds[1] = 1;
			inds[2] = 2;

			Mesh m = new Mesh();
			m.SetInds (inds);
			m.SetVerts(verts);
			return m;
		}

		#endregion
	}
}
