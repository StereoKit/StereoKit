// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;

class DemoUnicode : ITest
{
	string title       = "Unicode Text";
	string description = "";

	Model atlasModel;
	Model clipboard = Model.FromFile("Clipboard.glb", Shader.UI);

	Pose  clipboardPose  = (Demo.contentPose * Matrix.T(0.16f,-0.17f,0)).Pose;
	Pose  unicodeWinPose = (Demo.contentPose * Matrix.T(-0.16f,0,0)).Pose;

	string unicodeText = "";

	public void Initialize()
	{
		atlasModel = new Model(Mesh.Quad, TextStyle.Default.Material);
		atlasModel.RootNode.LocalTransform = Matrix.T(0,0,-0.01f);
		Tests.RunForFrames(2);
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		UI.HandleBegin("", ref clipboardPose, clipboard.Bounds);
		clipboard.Draw(Matrix.Identity);
		UI.LayoutArea(V.XY0(12, 15) * U.cm, V.XY(24, 30) * U.cm);
		UI.Label("Font Atlas");
		UI.HSeparator();
		UI.Model(atlasModel);
		UI.HandleEnd();

		UI.WindowBegin(" Unicode Samples", ref unicodeWinPose);
		UI.Label("Just type some Unicode here:");
		UI.Input("UnicodeText", ref unicodeText);
		UI.HSeparator();
		UI.Text("古池や\n蛙飛び込む\n水の音\n- Matsuo Basho");
		UI.HSeparator();
		UI.Text("Съешь же ещё этих мягких французских булок да выпей чаю. Широкая электрификация южных губерний даст мощный толчок подъёму сельского хозяйства. В чащах юга жил бы цитрус? Да, но фальшивый экземпляр!");
		UI.HSeparator();
		UI.Text("Windows only symbols:\n");
		UI.WindowEnd();

		Vec3 at = V.XYZ(0.65f, -.15f, -.35f);
		Tests.Screenshot("Unicode-ユニコード.jpg", 1, 600, 400, 90, at-V.XYZ(.19f,0,-.19f), at);

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0,-0.17f), V.XYZ(.7f, .44f, 0.1f)));
	}
}
