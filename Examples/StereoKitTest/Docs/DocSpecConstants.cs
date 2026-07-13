using StereoKit;

class DocSpecConstants : ITest
{
	/// :CodeSample: Material.SetInt Material.SetFloat Material.SetBool Material.SetUInt
	/// ### Specialization constants
	/// If a shader declares specialization constants with HLSL's
	/// `[[vk::constant_id(N)]]`, StereoKit exposes them through the ordinary
	/// scalar setters. The name you pass must match the HLSL variable name.
	///
	/// ```hlsl
	/// [[vk::constant_id(0)]] const int   COLOR_STEPS = 2;
	/// [[vk::constant_id(1)]] const float BRIGHTNESS  = 0.25;
	/// [[vk::constant_id(2)]] const bool  USE_TINT    = true;
	/// [[vk::constant_id(3)]] const uint  BLUE_SCALE  = 4;
	/// ```
	///
	/// Unlike a normal material parameter, a spec constant is baked into the
	/// shader pipeline. Setting one to a new value rebuilds that pipeline (a
	/// heavier operation than a uniform write), so prefer setting them at
	/// load time rather than every frame. Re-setting the same value is free.
	/// Two materials sharing one shader with different spec-constant values are
	/// distinct pipeline variants — handy for feature toggles or loop/array
	/// sizes the driver can then fold at compile time.
	Material variantA;
	Material variantB;
	public void Initialize()
	{
		Shader shader = Shader.FromFile("spec_constant_test.hlsl");

		variantA = new Material(shader); // uses the HLSL defaults

		variantB = new Material(shader);
		variantB.SetInt  ("COLOR_STEPS", 4);
		variantB.SetFloat("BRIGHTNESS",  0.6f);
		variantB.SetBool ("USE_TINT",    false);
		variantB.SetUInt ("BLUE_SCALE",  12);
	}
	/// :End:

	public void Shutdown() { }
	public void Step()
	{
		Mesh.Sphere.Draw(variantA, Matrix.TS(-0.15f, 0, -0.5f, 0.2f));
		Mesh.Sphere.Draw(variantB, Matrix.TS( 0.15f, 0, -0.5f, 0.2f));
	}
}
