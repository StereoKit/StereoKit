// This class animates a GLTF avatar from Ready Player Me based on user input!
// https://readyplayer.me/
//
// To use this tool, you can just add the class as a Stepper to your
// application, and reference the GLTF file of your avatar. It is intended
// primarily for use in first person, and may not meet all use-cases in its
// current state. This concept will be expanded on further in the future, so
// check back later for improvements to this tool!
//
// SK.AddStepper(new ReadyPlayerMeAvatar(Model.FromFile("avatar.glb")));

using System;

namespace StereoKit.Framework
{
	class ReadyPlayerMeAvatar : IStepper
	{
		Model        avatar;
		RenderLayer  layer;
		public bool  Enabled { get; private set; } = true;

		Vec3         headRootOffset;
		Vec3         eyeOffset;
		ModelNode    nodeHead;
		Vec3         nodeRotationOffset;
		ModelNode    nodeRotation;
		float        avatarHandSize;
		
		ModelNode[]  nodeHand     = new ModelNode[2];
		ModelNode[]  nodeElbow    = new ModelNode[2];
		ModelNode[]  nodeShoulder = new ModelNode[2];
		ModelNode[]  nodeEye      = new ModelNode[2];
		ModelNode[,] nodeHands    = new ModelNode[2, 15];

		const float shoulderWidth = 0.16f; // half the total shoulder width
		float forearm             = 0.22f; // length of the forearm
		float uarm                = 0.25f; // length of the upper arm
		const float headLength    = 0.1f;  // length from head point to neck
		const float neckLength    = 0.02f; // length from neck to center shoulders
		const float shoulderDrop  = 0.05f; // shoulder drop height from shoulder center
		const float elbowFlare    = 0.45f; // Elbows point at a location 1m down, and elbowFlare out from the shoulder

		/// <summary>Create an IStepper that will manage and draw an avatar
		/// driven by the user's input.</summary>
		/// <param name="avatarModel">A GLTF file from Ready Player Me. If null
		/// is provided here, then this class will attempt to load an
		/// "avatar.glb" from the Assets. If it fails, the IStepper will
		/// disable itself.</param>
		/// <param name="hideHead">Should the head of the avatar be drawn? This
		/// doesn't work with Ready Player Me's VR avatars, since they don't
		/// really distinguish between body parts.</param>
		/// <param name="renderLayer">What render layer mask will the avatar be
		/// drawn with? This is useful for first person viewpoints vs. cameras
		/// and hiding/showing the avatar.</param>
		public ReadyPlayerMeAvatar(Model avatarModel, bool hideHead = true, RenderLayer renderLayer = RenderLayer.Layer0)
		{
			if (avatarModel == null)
				avatarModel = Model.FromFile("avatar.glb");
			if (avatarModel == null)
			{
				Log.Warn("No valid avatar model was found!");
				Enabled = false;
				return;
			}
			
			avatar = avatarModel;
			layer  = renderLayer;

			// Find important nodes
			nodeHead     = avatar.FindNode("Head");
			nodeRotation = avatar.FindNode("Hips");

			// Find nodes for the left and right side of the body
			for (int i = 0; i < 2; i++)
			{
				string handStr = ((Handed)i).ToString();
				nodeHand    [i] = avatar.FindNode($"{handStr}Hand");
				nodeElbow   [i] = avatar.FindNode($"{handStr}ForeArm");
				nodeShoulder[i] = avatar.FindNode($"{handStr}Arm");
				nodeEye     [i] = avatar.FindNode($"{handStr}Eye");

				nodeHands[i, 0]      = avatar.FindNode($"{handStr}HandThumb1");
				nodeHands[i, 1]      = avatar.FindNode($"{handStr}HandThumb2");
				nodeHands[i, 2]      = avatar.FindNode($"{handStr}HandThumb3");

				nodeHands[i, 3  + 0] = avatar.FindNode($"{handStr}HandIndex1");
				nodeHands[i, 3  + 1] = avatar.FindNode($"{handStr}HandIndex2");
				nodeHands[i, 3  + 2] = avatar.FindNode($"{handStr}HandIndex3");

				nodeHands[i, 6  + 0] = avatar.FindNode($"{handStr}HandMiddle1");
				nodeHands[i, 6  + 1] = avatar.FindNode($"{handStr}HandMiddle2");
				nodeHands[i, 6  + 2] = avatar.FindNode($"{handStr}HandMiddle3");

				nodeHands[i, 9  + 0] = avatar.FindNode($"{handStr}HandRing1");
				nodeHands[i, 9  + 1] = avatar.FindNode($"{handStr}HandRing2");
				nodeHands[i, 9  + 2] = avatar.FindNode($"{handStr}HandRing3");

				nodeHands[i, 12 + 0] = avatar.FindNode($"{handStr}HandPinky1");
				nodeHands[i, 12 + 1] = avatar.FindNode($"{handStr}HandPinky2");
				nodeHands[i, 12 + 2] = avatar.FindNode($"{handStr}HandPinky3");
			}

			// Calculate the size of the hand so we can scale the hand mesh to
			// match our hand data. Note that for the first segment here, we're 
			// measuring from the root of the thumb to the root of the index
			// finger. This is becuase the index finger node starts at the
			// major knuckle, while the thumb root starts at the base of the
			// hand, so it's necessary for finding the right height.
			ModelNode indexTip = avatar.FindNode($"LeftHandIndex4");
			avatarHandSize =
				Vec3.Distance(nodeHands[0, 0].ModelTransform.Translation, nodeHands[0, 3].ModelTransform.Translation) +
				Vec3.Distance(nodeHands[0, 3].ModelTransform.Translation, nodeHands[0, 4].ModelTransform.Translation) +
				Vec3.Distance(nodeHands[0, 4].ModelTransform.Translation, nodeHands[0, 5].ModelTransform.Translation) +
				(indexTip == null
					? 0.034f
					: Vec3.Distance(nodeHands[0, 5].ModelTransform.Translation, indexTip.ModelTransform.Translation));


			// The head's base relative to the root node, this is used for
			// positioning the avatar via the head's position.
			headRootOffset     = nodeHead    .ModelTransform.Translation - avatar.RootNode.ModelTransform.Translation;
			nodeRotationOffset = nodeRotation.LocalTransform.Translation;
			
			// The center of the eyes, relative to the head's base.
			eyeOffset = (nodeEye[0].LocalTransform.Translation + nodeEye[1].LocalTransform.Translation) / 2.0f;

			// Find the length of the model's arms, used for calculating the
			// elbow position later on.
			if (nodeShoulder[0] != null && nodeElbow[0] != null) uarm    = Vec3.Distance(nodeShoulder[0].ModelTransform.Translation, nodeElbow[0].ModelTransform.Translation);
			if (nodeElbow   [0] != null && nodeHand [0] != null) forearm = Vec3.Distance(nodeElbow   [0].ModelTransform.Translation, nodeHand [0].ModelTransform.Translation);

			foreach (ModelNode node in avatar.Visuals)
			{
				// GLTF models often forget to cull backfaces, and
				// ReadyPlayerMe is no exception. This gains us fill-rate
				// performance
				node.Material.FaceCull = Cull.Back;
				
				// This can be used to hide the pants on full ReadyPlayerMe
				// avatars, but it often leaves behind bits of the body, so
				// it's not a full solution.
				if (node.Name.EndsWith("_Outfit_Bottom") ||
					node.Name.EndsWith("_Outfit_Footwear"))
				{
					//node.Visible = false;
				}

				// If we're inside the avatar, we don't want to render the head
				if (hideHead && (
					node.Name.EndsWith("_Head")    ||
					node.Name.EndsWith("_Hair")    ||
					node.Name.EndsWith("_Glasses") ||
					node.Name.EndsWith("_Teeth")   ||
					node.Name.Equals  ("EyeLeft")  ||
					node.Name.Equals  ("EyeRight")))
				{
					node.Visible = false;
				}
			}
		}

		public bool Initialize() => true;
		public void Shutdown() { }

		static Quat quatFlip         = Quat.FromAngles(0, 180, 0);
		static Quat quatCorrectNode  = Quat.FromAngles(90, 0, 180);
		static Quat quatCorrectThumb = Quat.FromAngles(90, 90, 270);
		public void Step()
		{
			Pose head     = Input.Head;
			head.position-= head.orientation * Quat.FromAngles(0, 180, 0) * eyeOffset;// V.XYZ(0, headHeight*0.4f, 0);
			Vec3 headBase = head.position + head.orientation * V.XYZ(0, -headLength, 0);
			Vec3 neck     = headBase - V.XYZ(0, neckLength, 0);

			// Shoulder forward facing direction is head direction weighted 
			// equally with the direction of both hands.
			Vec3 forward = 
				head.Forward.X0Z.Normalized
				+ (Input.Hand(Handed.Right).wrist.position - neck.X0Z).Normalized
				+ (Input.Hand(Handed.Left ).wrist.position - neck.X0Z).Normalized;
			forward = forward*0.25f;
			Vec3 right = Vec3.PerpendicularRight(forward, Vec3.Up).Normalized;

			// Position the avatar nodes
			avatar.RootNode.ModelTransform = Matrix.TR(head.position - headRootOffset, quatFlip);
			nodeRotation.LocalTransform    = Matrix.TR(nodeRotationOffset, Quat.LookDir(forward.X0Z));
			nodeHead.ModelTransform        = Matrix.TR(head.position, head.orientation * quatFlip);

			// Now for each arm
			for (int h = 0; h < 2; h++)
			{
				float handed  = h == 0 ? -1 : 1;
				Hand  hand    = Input.Hand((Handed)h);
				if (!hand.IsTracked) continue;

				float handScale = hand.size / avatarHandSize;
				Vec3  handPos   = hand.wrist.position;
				Vec3  shoulder  = neck + right*handed*shoulderWidth - Vec3.Up*shoulderDrop;

				// Triangle represented by 3 edges, forearm, uarm, and armDist
				float armDist = Math.Min(forearm+uarm, Vec3.Distance(shoulder, handPos));

				// Heron's formula to find area
				float s       = (forearm + uarm + armDist) / 2;
				float area    = SKMath.Sqrt(s*(s-forearm)*(s-uarm)*(s-armDist));
				// Height of triangle based on area
				float offsetH = (2*area)/armDist;
				// Height can now be used to calculate how far off the elbow is
				float offsetD = SKMath.Sqrt(Math.Abs(offsetH * offsetH - uarm * uarm));
				
				// Elbow calculation begins somewhere along the line between the
				// shoulder and the wrist.
				Vec3 dir = (handPos - shoulder).Normalized;
				Vec3 at  = shoulder + dir * offsetD;
				// The elbow naturally flares out to the side, rather than 
				// dropping straight down. Here, we find a point to flare out 
				// towards.
				Vec3  flarePoint = neck + right*handed*elbowFlare - Vec3.Up;
				Plane flarePlane = new Plane(flarePoint, Ray.FromTo(neck, handPos).Closest(flarePoint)-flarePoint);
				Vec3  dirDown    = (flarePlane.Closest(at)-at).Normalized;
				Vec3  elbow      = at + dirDown * offsetH;
				
				// Place the shoulder and elbow nodes
				if (nodeShoulder[h] != null) nodeShoulder[h].ModelTransform = Matrix.TR(shoulder, Quat.LookAt(shoulder, elbow) * quatCorrectNode);
				if (nodeElbow   [h] != null) nodeElbow   [h].ModelTransform = Matrix.TR(elbow,    Quat.LookAt(elbow, hand.wrist.position, Vec3.Lerp(hand.wrist.Up, Vec3.Up, 0.5f)) * quatCorrectNode);

				// The wrist, there may be a bug in the simulator wrist code
				// at the moment.
				Pose handPose = hand.wrist;
				if (nodeHand[h] != null)
					nodeHand[h].ModelTransform = Matrix.TRS(handPose.position, handPose.orientation * (Backend.XRType == BackendXRType.Simulator?quatFlip:quatCorrectNode), handScale);
				
				// And finally, the hand joints
				for (int j = 0; j < 15; j++)
				{
					Pose p = hand.Get(j/3, 1+(j%3)).Pose;
					p.orientation = p.orientation * ((j/3==0) ? quatCorrectThumb : quatCorrectNode);
					nodeHands[h, j].ModelTransform = p.ToMatrix(handScale);
				}
			}

			avatar.Draw(Matrix.Identity, Color.White, layer);
		}
	}
}