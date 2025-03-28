using StereoKit;

class TestSprite : ITest
{
	Sprite sprite = Sprite.FromFile("StereoKitWide.png", SpriteType.Single);

	public void Initialize() { }
	public void Shutdown() { }
	public void Step()
	{
		Tests.Screenshot("Tests/SpriteAlign.jpg", 800, 300, 52, V.XYZ(0, 0, -0.20f), V.XYZ(0, 0, 0));

		Lines.Add(V.XYZ(-1,    0,    0), V.XYZ( 1,   0,    0), new Color32(255,0,0,255), 0.001f);
		Lines.Add(V.XYZ(-1,    0.05f,0), V.XYZ( 1,   0.05f,0), new Color32(255,0,0,128), 0.001f);
		Lines.Add(V.XYZ(-1,   -0.05f,0), V.XYZ( 1,  -0.05f,0), new Color32(255,0,0,128), 0.001f);
		Lines.Add(V.XYZ( 0,   -1,    0), V.XYZ( 0,   1,    0), new Color32(0,255,0,255), 0.001f);
		Lines.Add(V.XYZ( 0.1f,-1,    0), V.XYZ( 0.1f,1,    0), new Color32(0,255,0,128), 0.001f);
		Lines.Add(V.XYZ(-0.1f,-1,    0), V.XYZ(-0.1f,1,    0), new Color32(0,255,0,128), 0.001f);
		
		sprite.Draw(Matrix.TS( 0.1f, 0.05f, 0, 0.03f), Pivot.BottomRight);  // Left
		sprite.Draw(Matrix.TS( 0,    0.05f, 0, 0.03f), Pivot.BottomCenter); // Center
		sprite.Draw(Matrix.TS(-0.1f, 0.05f, 0, 0.03f), Pivot.BottomLeft);   // Right
		
		sprite.Draw(Matrix.TS( 0.1f, 0, 0, 0.03f), Pivot.CenterRight);      // Left
		sprite.Draw(Matrix.S ( 0.03f),             Pivot.Center);           // Center
		sprite.Draw(Matrix.TS(-0.1f, 0, 0, 0.03f), Pivot.CenterLeft);       // Right

		sprite.Draw(Matrix.TS( 0.1f, -0.05f, 0, 0.03f), Pivot.TopRight);    // Left
		sprite.Draw(Matrix.TS( 0,    -0.05f, 0, 0.03f), Pivot.TopCenter);   // Center
		sprite.Draw(Matrix.TS(-0.1f, -0.05f, 0, 0.03f), Pivot.TopLeft);     // Right
	}
}
