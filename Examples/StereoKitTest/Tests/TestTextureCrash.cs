using StereoKit;

namespace StereoKitTest
{
	public class TestTextureCrash : ITest
	{
		private Model _model  = Model.FromMesh(Mesh.GenerateCylinder(.10f, 1, Vec3.Up), Default.Material.Copy());
		private bool  _doOnce = false;

		public void Step()
		{
			if (!_doOnce)
			{
				Material mat = _model.RootNode.Material;
				mat.SetTexture("diffuse", Tex.FromFile("floor.png"));

				_doOnce = true;
			}
			_model.Draw(Matrix.T(Vec3.Zero));
		}

		public void Initialize() { Tests.RunForFrames(2); }
		public void Shutdown  () { }
	}
}
