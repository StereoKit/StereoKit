using StereoKit;

class TestSprite : ITest
{
	Sprite sprite = Sprite.FromFile("StereoKitWide.png", SpriteType.Single);

	public void Initialize() { }
	public void Shutdown() { }
	public void Update()
	{
		Tests.Screenshot("Tests/SpriteAlign.jpg", 800, 300, 52, V.XYZ(0, 0, -0.20f), V.XYZ(0, 0, 0));

		Lines.Add(V.XYZ(-1,    0,    0), V.XYZ( 1,   0,    0), new Color32(255,0,0,255), 0.001f);
		Lines.Add(V.XYZ(-1,    0.05f,0), V.XYZ( 1,   0.05f,0), new Color32(255,0,0,128), 0.001f);
		Lines.Add(V.XYZ(-1,   -0.05f,0), V.XYZ( 1,  -0.05f,0), new Color32(255,0,0,128), 0.001f);
		Lines.Add(V.XYZ( 0,   -1,    0), V.XYZ( 0,   1,    0), new Color32(0,255,0,255), 0.001f);
		Lines.Add(V.XYZ( 0.1f,-1,    0), V.XYZ( 0.1f,1,    0), new Color32(0,255,0,128), 0.001f);
		Lines.Add(V.XYZ(-0.1f,-1,    0), V.XYZ(-0.1f,1,    0), new Color32(0,255,0,128), 0.001f);
		
		sprite.Draw(Matrix.TS( 0.1f, 0.05f, 0, 0.03f), TextAlign.BottomRight);  // Left
		sprite.Draw(Matrix.TS( 0,    0.05f, 0, 0.03f), TextAlign.BottomCenter); // Center
		sprite.Draw(Matrix.TS(-0.1f, 0.05f, 0, 0.03f), TextAlign.BottomLeft);   // Right
		
		sprite.Draw(Matrix.TS( 0.1f, 0, 0, 0.03f), TextAlign.CenterRight);      // Left
		sprite.Draw(Matrix.S ( 0.03f),             TextAlign.Center);           // Center
		sprite.Draw(Matrix.TS(-0.1f, 0, 0, 0.03f), TextAlign.CenterLeft);       // Right

		sprite.Draw(Matrix.TS( 0.1f, -0.05f, 0, 0.03f), TextAlign.TopRight);    // Left
		sprite.Draw(Matrix.TS( 0,    -0.05f, 0, 0.03f), TextAlign.TopCenter);   // Center
		sprite.Draw(Matrix.TS(-0.1f, -0.05f, 0, 0.03f), TextAlign.TopLeft);     // Right
	}
}
