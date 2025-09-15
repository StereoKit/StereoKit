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
		for (int i = 0; i < (int)PermissionType.Max; i++)
		{
			UI.PushId(i);
			PermissionType  permission = (PermissionType)i;
			PermissionState state      = Permission.State(permission);

			UI.Label(permission.ToString(), labelSize);
			UI.SameLine();
			UI.Label(state.ToString(), labelSize);
			UI.SameLine();
			UI.PushEnabled(state == PermissionState.Capable && Permission.IsInteractive(permission));
			if (UI.Button("Request")) Permission.Request(permission);
			UI.PopEnabled();
			UI.PopId();
		}
		UI.WindowEnd();
	}
}