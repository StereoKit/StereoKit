using System;

namespace StereoKit.Framework
{
	class AvatarSkeleton : IStepper
	{
		LinePoint[] headLine = new LinePoint[3];
		LinePoint[] armLine  = new LinePoint[4];

		public bool Enabled => true;

		public bool Initialize()
		{
			for (int i = 0; i < headLine.Length; i++)
			{
				headLine[i].color     = Color32.White;
				headLine[i].thickness = 0.01f;
			}
			for (int i = 0; i < 4; i++)
			{
				armLine[i].color     = Color32.White;
				armLine[i].thickness = 0.01f;
			}
			return true;
		}

		public void Shutdown()
		{
		}

		public void Step()
		{
			const float shoulderWidth = 0.16f; // half the total shoulder width
			const float forearm       = 0.22f; // length of the forearm
			const float uarm          = 0.25f; // length of the upper arm
			const float headLength    = 0.1f;  // length from head point to neck
			const float neckLength    = 0.02f; // length from neck to center shoulders
			const float shoulderDrop  = 0.05f; // shoulder drop height from shoulder center
			const float elbowFlare    = 0.45f; // Elbows point at a location 1m down, and elbowFlare out from the shoulder

			Pose head  = Input.Head;

			// Head, neck, and shoulder center
			headLine[0].pt = head.position;
			headLine[1].pt = headLine[0].pt + head.orientation*V.XYZ(0,-headLength, 0);
			headLine[2].pt = headLine[1].pt - V.XYZ(0,neckLength, 0);
			headLine[0].pt = Vec3.Lerp(headLine[0].pt, headLine[1].pt, 0.1f);
			Lines.Add(headLine);

			// Shoulder forward facing direction is head direction weighted 
			// equally with the direction of both hands.
			Vec3 forward = 
				head.Forward.X0Z.Normalized*2 
				+ (Input.Hand(Handed.Right).wrist.position-headLine[2].pt.X0Z).Normalized
				+ (Input.Hand(Handed.Left ).wrist.position-headLine[2].pt.X0Z).Normalized;
			forward = forward*0.25f;
			Vec3 right = Vec3.PerpendicularRight(forward, Vec3.Up).Normalized;

			// Now for each arm
			for (int h = 0; h < 2; h++)
			{
				float handed  = h == 0 ? -1 : 1;
				Hand  hand    = Input.Hand((Handed)h);
				Vec3  handPos = hand.wrist.position;
				if (!hand.IsTracked) continue;

				armLine[0].pt = headLine[2].pt;
				armLine[1].pt = armLine[0].pt + right*handed*shoulderWidth - Vec3.Up*shoulderDrop;
				armLine[3].pt = handPos;

				// Triangle represented by 3 edges, forearm, uarm, and armDist
				float armDist = Math.Min(forearm+uarm,Vec3.Distance(armLine[1].pt, handPos));

				// Heron's formula to find area
				float s       = (forearm + uarm + armDist) / 2;
				float area    = SKMath.Sqrt(s*(s-forearm)*(s-uarm)*(s-armDist));
				// Height of triangle based on area
				float offsetH = (2*area)/armDist;
				// Height can now be used to calculate how far off the elbow is
				float offsetD = SKMath.Sqrt(Math.Abs(offsetH * offsetH - uarm * uarm));

				// Elbow calculation begins somewhere along the line between the
				// shoulder and the wrist.
				Vec3 dir = (handPos - armLine[1].pt).Normalized;
				Vec3 at  = armLine[1].pt + dir * offsetD;
				// The elbow naturally flares out to the side, rather than 
				// dropping straight down. Here, we find a point to flare out 
				// towards.
				Vec3  flarePoint = headLine[2].pt + right*handed*elbowFlare - Vec3.Up;
				Plane flarePlane = new Plane(flarePoint, Ray.FromTo(headLine[2].pt, handPos).Closest(flarePoint)-flarePoint);
				Vec3  dirDown    = (flarePlane.Closest(at)-at).Normalized;
				armLine[2].pt = at + dirDown * offsetH;

				Lines.Add(armLine);
				for (int i = 1; i < 5; i++)
					Lines.Add(handPos, hand[(FingerId)i, JointId.KnuckleMajor].position, Color32.White, new Color32(255,255,255,0), 0.01f);
			}
		}
	}
}
