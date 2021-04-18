using StereoKit;
using System;

class DemoElbowEst : ITest
{
	LinePoint[] headLine = new LinePoint[3];
	LinePoint[] armLine  = new LinePoint[4];
	public void Initialize()
	{
		for (int i = 0; i < headLine.Length; i++)
		{
			headLine[i].color = Color32.White;
			headLine[i].thickness = 0.01f;
		}
		for (int i = 0; i < 4; i++)
		{
			armLine[i].color = Color32.White;
			armLine[i].thickness = 0.01f;
		}
	}

	public void Update()
	{
		Pose head  = Input.Head;
		Vec3 right = Vec3.PerpendicularRight(head.Forward, Vec3.Up);

		// Head and shoulders
		headLine[0].pt = head.position;
		headLine[1].pt = headLine[0].pt + head.orientation*V.XYZ(0,-.1f,0);
		headLine[2].pt = headLine[1].pt - V.XYZ(0,.1f,0);
		Lines.Add(headLine);

		// Now for each arm
		for (int h = 0; h < 2; h++)
		{
			float handed  = h == 0 ? -1 : 1;
			Hand  hand    = Input.Hand((Handed)h);
			Vec3  handPos = hand.wrist.position;
			if (!hand.IsTracked) continue;

			armLine[0].pt = headLine[2].pt;
			armLine[1].pt = armLine[0].pt + right*handed*.2f - Vec3.Up*0.05f;
			armLine[3].pt = handPos;

			// Triangle represented by 3 edges, forearm, uarm, and armDist
			const float forearm = .25f;
			const float uarm    = .28f;
			float       armDist = Math.Min(forearm+uarm,Vec3.Distance(armLine[1].pt, handPos));

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
			// There are many potential elbows, but we'll pick one that's
			// straight down. A better guess might flare the elbows out a bit
			// more.
			Vec3 dirRight = Vec3.PerpendicularRight(dir, Vec3.Up);
			Vec3 dirDown  = Vec3.PerpendicularRight(dir, dirRight);
			armLine[2].pt = at + dirDown * offsetH;

			Lines.Add(armLine);
			for (int i = 1; i < 5; i++)
				Lines.Add(handPos, hand[(FingerId)i, JointId.KnuckleMajor].position, Color32.White, new Color32(255,255,255,0), 0.01f);
		}
	}

	public void Shutdown()
	{ 
	}
}