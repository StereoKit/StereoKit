using StereoKit;

class DemoPermissions : ITest
{
	public void Initialize()
	{
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		UI.WindowBegin("Permissions");
		Vec2 labelSize = new Vec2(0.1f, 0);
		for (int i = 0; i < (int)Permission.Max; i++)
		{
			UI.PushId(i);
			Permission      permission = (Permission)i;
			PermissionState state      = Permissions.State(permission);

			UI.Label(permission.ToString(), labelSize);
			UI.SameLine();
			UI.Label(state.ToString(), labelSize);
			UI.SameLine();
			UI.PushEnabled(state == PermissionState.Capable && Permissions.IsInteractive(permission));
			if (UI.Button("Request")) Permissions.Request(permission);
			UI.PopEnabled();
			UI.PopId();
		}
		UI.WindowEnd();
	}
}