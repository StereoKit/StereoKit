using StereoKit;

class DocTexGenParticle: ITest
{
	Sprite[] spriteList;

	public void Initialize()
	{
		/// :CodeSample: Tex.GenParticle Sprite.FromTex
		/// ### Generating Particle Sprites
		/// Sometimes you just need a small blob of color for visual effects or
		/// other things! Instead of firing up an image editor, you can just
		/// use Tex.GenParticle!
		/// 
		/// This sample generates a number of different shapes defined by the
		/// `roundness` parameter. Starting at 0, and increasing at .1
		/// increments to 1.0.
		Sprite[] sprites = new Sprite[10];
		for (int i = 0; i < sprites.Length; i++)
		{
			float roundness   = i / (float)(sprites.Length - 1);
			Tex   particleTex = Tex.GenParticle(64, 64, roundness);
			sprites[i] = Sprite.FromTex(particleTex, SpriteType.Single);
		}
		// :End:

		spriteList = sprites;
	}


	public void Update() {
		Hierarchy.Push(Matrix.T(0,4,2));

		Sprite[] sprites = spriteList;

		/// :CodeSample: Tex.GenParticle Sprite.FromTex
		/// And here's what that looks like when you draw using this code!
		for (int i = 0; i < sprites.Length; i++)
			sprites[i].Draw(Matrix.TS(V.XY0(i%5, -i/5)*0.1f, 0.1f));
		/// ![Generated particle sprites]({{site.screen_url}}/TexGenParticles.jpg)
		/// :End:

		Tests.Screenshot("TexGenParticles.jpg", 600, 300, Hierarchy.ToWorld(new Vec3(0.25f, -0.1f, 0.15f)), Hierarchy.ToWorld(new Vec3(0.25f, -0.1f, 0)));

		Hierarchy.Pop();
	}

	public void Shutdown() { }
}