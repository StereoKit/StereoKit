using StereoKit;
using System.IO;
using System.Linq;

class TestGltf : ITest
{
	string[] _files     = null;
	int      _currFile  = 0;
	Model    _currModel = null;
	int      _frames = 1;

	public void Initialize()
	{
		if (Tests.GltfFolders == null) return;
		
		_files = Tests.GltfFolders
			.Split(';')
			.SelectMany(folder => new string[] { "*.glb", "*.gltf" }
				.SelectMany(ext => Directory.EnumerateFiles(folder, ext, SearchOption.AllDirectories)))
			.ToArray();
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		if (_files == null) return;

		_frames += 1;
		Tests.RunForFrames(_frames);

		if (_currModel != null)
		{
			float scale = 1 / _currModel.Bounds.dimensions.Magnitude;
			Matrix t = Matrix.TRS(
				V.XYZ(0, -0.25f, -0.5f),
				Quat.LookDir(0, 0, 1), scale);
			Hierarchy.Push(t);
			_currModel.Draw(Matrix.T(-_currModel.Bounds.center));
			Mesh.Cube.Draw(Material.UIBox, Matrix.S(_currModel.Bounds.dimensions));
			Hierarchy.Pop();
		}

		bool isFinished = Assets.CurrentTask == Assets.TotalTasks;
		if (isFinished)
		{
			if (Tests.GltfScreenshotRoot != null)
				Tests.ScreenshotGltf(_files[_currFile].Replace('/', '_').Replace('\\','_').Replace(':','_') + ".jpg", 500, 500, V.XYZ(0,-0.25f,1.0f), V.XYZ(0,-0.25f,0));
			Log.Info($"Loading {_files[_currFile]}");

			if (_currFile + 1 >= _files.Length)
				_files = null;
			else
			{
				_currModel = Model.FromFile(_files[_currFile]);
				_currFile += 1;
			}
		}
	}
}
