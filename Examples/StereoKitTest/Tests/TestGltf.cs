using StereoKit;
using System.IO;
using System.Linq;

class TestGltf : ITest
{
	enum State
	{
		None,
		Loading,
		View,
		Screenshot,
	}

	string[] _files     = null;
	int      _currFile  = 0;
	Model    _currModel = null;
	int      _frames    = 1;
	State    _state     = State.None;
	int      _frameCounter = 0;

	public void Initialize()
	{
		if (Tests.GltfFolders == null) return;
		
		_files = Tests.GltfFolders
			.Split(';')
			.SelectMany(folder => new string[] { "*.glb", "*.gltf" }
				.SelectMany(ext => Directory.EnumerateFiles(folder, ext, SearchOption.AllDirectories)))
			.Where(file => !file.Contains("glTF-Draco"))
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

		if (_state == State.None)
		{
			Log.Info($"Loading {_files[_currFile]}");
			_currModel = Model.FromFile(_files[_currFile]);
			_currFile += 1;
			_state     = State.Loading;
		}
		else if (_state == State.Loading)
		{
			if (Assets.CurrentTask == Assets.TotalTasks)
			{
				_state        = State.View;
				_frameCounter = 0;
			}
		}
		else if (_state == State.View)
		{
			_frameCounter += 1;
			if (_frameCounter >= 4)
			{
				_state        = State.Screenshot;
				_frameCounter = 0;
			}
		}
		else if (_state == State.Screenshot)
		{
			_frameCounter += 1;
			if (_frameCounter == 2 && Tests.GltfScreenshotRoot != null)
			{
				Tests.ScreenshotGltf(_files[_currFile-1].Replace('/', '_').Replace('\\', '_').Replace(':', '_') + ".jpg", 1000, 1000, V.XYZ(0, -0.25f, 1.0f), V.XYZ(0, -0.25f, 0));
			}
			if (_frameCounter >= 4)
			{
				if (_currFile + 1 >= _files.Length)
					_files = null;
				_state = State.None;
			}
		}
	}
}
