using StereoKit;
using System;

class TestAnimations : ITest
{
	Model _srcModel;
	Model _anim1;
	Model _anim2;

	public void Initialize()
	{
		_srcModel = Model.FromFile("Cosmonaut.glb");
		_anim1 = _srcModel.Copy();
		_anim1.PlayAnim("Idle", AnimMode.Loop);
		_anim2 = _anim1.Copy();
		_anim2.PlayAnim("Jump", AnimMode.Loop);

		Tests.RunForSeconds(Math.Min(_anim1.ActiveAnim.Duration, _anim2.ActiveAnim.Duration));
	}

	public void Shutdown()
	{
	}

	public void Update()
	{
		_srcModel.Draw(Matrix.TR(-1, -1.3f, -1, Quat.LookDir(-Vec3.Forward)));
		_anim1   .Draw(Matrix.TR( 0, -1.3f, -1, Quat.LookDir(-Vec3.Forward)));
		_anim2   .Draw(Matrix.TR( 1, -1.3f, -1, Quat.LookDir(-Vec3.Forward)));

		Tests.Screenshot("Tests/MultiAnimation.jpg", 10, 600, 400, 90, V.XYZ(0, -.3f, .5f), V.XYZ(0, -.3f, 0));
	}
}