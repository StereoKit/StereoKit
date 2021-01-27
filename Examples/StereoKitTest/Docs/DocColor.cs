using StereoKit;

class DocColor : ITest
{
	public void DocsColor128()
	{
		/// :CodeSample: Color Color.HSV
		// You can create a color using Red, Green, Blue, Alpha values,
		// but it's often a great recipe for making a bad color.
		Color color = new Color(1,0,0,1); // Red

		// Hue, Saturation, Value, Alpha is a more natural way of picking
		// colors. The commentdocs have a list of important values for Hue,
		// to make it a little easier to pick the hue you want.
		color = Color.HSV(0, 1, 1, 1); // Red

		// And there's a few static colors available if you need 'em.
		color = Color.White;

		// You can also implicitly convert Color to a Color32!
		Color32 color32 = color;
		/// :End:

		/// :CodeSample: Color.HSV Color.ToHSV Color.LAB Color.ToLAB
		// Desaturating a color can be done quite nicely with the HSV
		// functions
		Color red      = new Color(1,0,0,1);
		Vec3  colorHSV = red.ToHSV();
		colorHSV.y *= 0.5f; // Drop saturation by half
		Color desaturatedRed = Color.HSV(colorHSV, red.a);

		// LAB color space is excellent for modifying perceived 
		// brightness, or 'Lightness' of a color.
		Color green    = new Color(0,1,0,1);
		Vec3  colorLAB = green.ToLAB();
		colorLAB.x *= 0.5f; // Drop lightness by half
		Color darkGreen = Color.LAB(colorLAB, green.a);
		/// :End:
	}

	public void DocsColor32()
	{
		/// :CodeSample: Color32
		// You can create a color using Red, Green, Blue, Alpha values,
		// but it's often a great recipe for making a bad color.
		Color32 color = new Color32(255, 0, 0, 255); // Red

		// Hue, Saturation, Value, Alpha is a more natural way of picking
		// colors. You can use Color's HSV function, plus the implicit
		// conversion to make a Color32!
		color = Color.HSV(0, 1, 1, 1); // Red

		// And there's a few static colors available if you need 'em.
		color = Color32.White;
		/// :End:
	}

	public void Initialize()
	{
		DocsColor128();
		DocsColor32();
	}

	public void Shutdown() { }
	public void Update() { }
}