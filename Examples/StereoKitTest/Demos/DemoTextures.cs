using StereoKit;

class DemoTextures : ITest
{
	Material exampleMaterial = Material.Default.Copy();
	Mesh     quad            = Mesh.GeneratePlane(new Vec2(.4f,.4f), -Vec3.Forward, Vec3.Up);

	Matrix   descPose    = Matrix.TR (-0.5f, 0, -0.5f, Quat.LookDir(1,0,1));
	string   description = "Here's a quick sample of procedurally assembling a texture!";
	Matrix   titlePose   = Matrix.TRS(V.XYZ(-0.5f, 0.05f, -0.5f), Quat.LookDir(1, 0, 1), 2);
	string   title       = "Procedural Textures";

	public void Initialize()
	{
		/// :CodeSample: Tex.SetColors
		/// ### Creating a texture procedurally
		/// It's pretty easy to create an array of colors, and
		/// just pass that into an empty texture! Here, we're 
		/// building a simple grid texture, like so:
		/// 
		/// ![Procedural Texture]({{site.url}}/img/screenshots/ProceduralTexture.jpg)
		/// 
		/// You can call SetTexture as many times as you like! If 
		/// you're calling it frequently, you may want to keep
		/// the width and height consistent to prevent from creating
		/// new texture objects. Use TexType.ImageNomips to prevent
		/// StereoKit from calculating mip-maps, which can be costly,
		/// especially when done frequently.
		// Create an empty texture! This is TextType.Image, and 
		// an RGBA 32 bit color format.
		Tex gridTex = new Tex();

		// Use point sampling to ensure that the grid lines are
		// crisp and sharp, not blended with the pixels around it.
		gridTex.SampleMode = TexSample.Point;

		// Allocate memory for the pixels we'll fill in, powers
		// of two are always best for textures, since this makes
		// things like generating mip-maps easier.
		int width  = 128;
		int height = 128;
		Color32[] colors = new Color32[width*height];

		// Create a color for the base of the grid, and the
		// lines of the grid
		Color32 baseColor    = Color.HSV(0.6f,0.1f,0.25f);
		Color32 lineColor    = Color.HSV(0.6f,0.05f,1);
		Color32 subLineColor = Color.HSV(0.6f,0.05f,.6f);

		// Loop through each pixel
		for (int y = 0; y < height; y++) {
		for (int x = 0; x < width;  x++) {
			// If the pixel's x or y value is a multiple of 64, or 
			// if it's adjacent to a multiple of 128, then we 
			// choose the line color! Otherwise, we use the base.
			if (x % 128 == 0 || (x+1)%128 == 0 || (x-1)%128 == 0 ||
				y % 128 == 0 || (y+1)%128 == 0 || (y-1)%128 == 0)
				colors[x+y*width] = lineColor;
			else if (x % 64 == 0 || y % 64 == 0)
				colors[x+y*width] = subLineColor;
			else
				colors[x+y*width] = baseColor;
		} }

		// Put the pixel information into the texture
		gridTex.SetColors(width, height, colors);
		/// :End:

		/// :CodeSample: Material MatParamName
		/// ### Material parameter access
		/// Material does have an array operator overload for setting 
		/// shader parameters really quickly! You can do this with strings
		/// representing shader parameter names, or use the MatParamName
		/// enum for compile safety.
		exampleMaterial[MatParamName.DiffuseTex] = gridTex;
		exampleMaterial[MatParamName.TexScale  ] = 2.0f;
		/// :End:
	}

	public void Shutdown() { }

	public void Update()
	{
		Tests.Screenshot("ProceduralTexture.jpg", 600, 600, new Vec3(0.32f, 0, -0.32f), new Vec3(0.5f, 0, -0.5f));

		quad.Draw(exampleMaterial, Matrix.TR(0.5f, 0, -0.5f, Quat.LookDir(1, 0, -1)));

		Text.Add(title, titlePose);
		Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
	}
}
