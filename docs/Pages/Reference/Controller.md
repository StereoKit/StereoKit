---
layout: default
title: Controller
description: This represents a physical controller input device! Tracking information, buttons, analog sticks and triggers! There's also a Menu button that's tracked separately at Input.ContollerMenu.
---
# class Controller

This represents a physical controller input device! Tracking
information, buttons, analog sticks and triggers! There's also a Menu
button that's tracked separately at Input.ContollerMenu.

## Instance Fields and Properties

|  |  |
|--|--|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) [aim]({{site.url}}/Pages/Reference/Controller/aim.html)|The aim pose of a controller is where the controller 'points' from and to. This is great for pointer rays and far interactions.|
|float [grip]({{site.url}}/Pages/Reference/Controller/grip.html)|The grip button typically sits under the user's middle finger. These buttons occasionally have a wide range of activation, so this is provided as a value from 0.0 -> 1.0, where 0 is no interaction, and 1 is full interaction. If a controller has binary activation, this will jump straight from 0 to 1.|
|bool [IsJustTracked]({{site.url}}/Pages/Reference/Controller/IsJustTracked.html)|Has the controller just started being tracked this frame?|
|bool [IsJustUntracked]({{site.url}}/Pages/Reference/Controller/IsJustUntracked.html)|Has the controller just stopped being tracked this frame?|
|bool [IsStickClicked]({{site.url}}/Pages/Reference/Controller/IsStickClicked.html)|Is the analog stick/directional controller button currently being actively pressed?|
|bool [IsStickJustClicked]({{site.url}}/Pages/Reference/Controller/IsStickJustClicked.html)|Has the analog stick/directional controller button just been pressed this frame?|
|bool [IsStickJustUnclicked]({{site.url}}/Pages/Reference/Controller/IsStickJustUnclicked.html)|Has the analog stick/directional controller button just been released this frame?|
|bool [IsTracked]({{site.url}}/Pages/Reference/Controller/IsTracked.html)|Is the controller being tracked by the sensors right now?|
|bool [IsX1JustPressed]({{site.url}}/Pages/Reference/Controller/IsX1JustPressed.html)|Has the controller's X1 button just been pressed this frame? Depending on the specific hardware, this is the first general purpose button on the controller. For example, on an Oculus Quest Touch controller this would represent 'X' on the left controller, and 'A' on the right controller.|
|bool [IsX1JustUnPressed]({{site.url}}/Pages/Reference/Controller/IsX1JustUnPressed.html)|Has the controller's X1 button just been released this frame? Depending on the specific hardware, this is the first general purpose button on the controller. For example, on an Oculus Quest Touch controller this would represent 'X' on the left controller, and 'A' on the right controller.|
|bool [IsX1Pressed]({{site.url}}/Pages/Reference/Controller/IsX1Pressed.html)|Is the controller's X1 button currently pressed? Depending on the specific hardware, this is the first general purpose button on the controller. For example, on an Oculus Quest Touch controller this would represent 'X' on the left controller, and 'A' on the right controller.|
|bool [IsX2JustPressed]({{site.url}}/Pages/Reference/Controller/IsX2JustPressed.html)|Has the controller's X2 button just been pressed this frame? Depending on the specific hardware, this is the second general purpose button on the controller. For example, on an Oculus Quest Touch controller this would represent 'X' on the left controller, and 'A' on the right controller.|
|bool [IsX2JustUnPressed]({{site.url}}/Pages/Reference/Controller/IsX2JustUnPressed.html)|Has the controller's X2 button just been released this frame? Depending on the specific hardware, this is the second general purpose button on the controller. For example, on an Oculus Quest Touch controller this would represent 'X' on the left controller, and 'A' on the right controller.|
|bool [IsX2Pressed]({{site.url}}/Pages/Reference/Controller/IsX2Pressed.html)|Is the controller's X2 button currently pressed? Depending on the specific hardware, this is the second general purpose button on the controller. For example, on an Oculus Quest Touch controller this would represent 'X' on the left controller, and 'A' on the right controller.|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) [pose]({{site.url}}/Pages/Reference/Controller/pose.html)|The grip pose of the controller. This approximately represents the center of the hand's position. Check `trackedPos` and `trackedRot` for the current state of the pose data.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [stick]({{site.url}}/Pages/Reference/Controller/stick.html)|This is the current 2-axis position of the analog stick or equivalent directional controller. This generally ranges from -1 to +1 on each axis. This is a raw input, so dead-zones and similar issues are not accounted for here, unless modified by the OpenXR platform itself.|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [stickClick]({{site.url}}/Pages/Reference/Controller/stickClick.html)|This represents the click state of the controller's analog stick or directional controller.|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [tracked]({{site.url}}/Pages/Reference/Controller/tracked.html)|This tells the current tracking state of this controller overall. If either position or rotation are trackable, then this will report tracked. Typically, positional tracking will be lost first, when the controller goes out of view, and rotational tracking will often remain as long as the controller is still connected. This is a good way to check if the controller is connected to the system at all.|
|[TrackState]({{site.url}}/Pages/Reference/TrackState.html) [trackedPos]({{site.url}}/Pages/Reference/Controller/trackedPos.html)|This tells the current tracking state of the controller's position information. This is often the first part of tracking data to go, so it can often be good to account for this on occasions.|
|[TrackState]({{site.url}}/Pages/Reference/TrackState.html) [trackedRot]({{site.url}}/Pages/Reference/Controller/trackedRot.html)|This tells the current tracking state of the controller's rotational information.|
|float [trigger]({{site.url}}/Pages/Reference/Controller/trigger.html)|The trigger button at the user's index finger. These buttons typically have a wide range of activation, so this is provided as a value from 0.0 -> 1.0, where 0 is no interaction, and 1 is full interaction. If a controller has binary activation, this will jump straight from 0 to 1.|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [x1]({{site.url}}/Pages/Reference/Controller/x1.html)|The current state of the controller's X1 button. Depending on the specific hardware, this is the first general purpose button on the controller. For example, on an Oculus Quest Touch controller this would represent 'X' on the left controller, and 'A' on the right controller.|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [x2]({{site.url}}/Pages/Reference/Controller/x2.html)|The current state of the controller's X2 button. Depending on the specific hardware, this is the second general purpose button on the controller. For example, on an Oculus Quest Touch controller this would represent 'Y' on the left controller, and 'B' on the right controller.|

## Examples

### Controller Debug Visualizer
This function shows a debug visualization of the current state of
the controller! It's not something you'd show to users, but it's
nice for just seeing how the API works, or as a temporary
visualization.
```csharp
void ShowController(Handed hand)
{
	Controller c = Input.Controller(hand);
	if (!c.IsTracked) return;

	Hierarchy.Push(c.pose.ToMatrix());
		// Pick the controller color based on trackin info state
		Color color = Color.Black;
		if (c.trackedPos == TrackState.Inferred) color.g = 0.5f;
		if (c.trackedPos == TrackState.Known)    color.g = 1;
		if (c.trackedRot == TrackState.Inferred) color.b = 0.5f;
		if (c.trackedRot == TrackState.Known)    color.b = 1;
		Default.MeshCube.Draw(Default.Material, Matrix.S(new Vec3(3, 3, 8) * U.cm), color);

		// Show button info on the back of the controller
		Hierarchy.Push(Matrix.TR(0,1.6f*U.cm,0, Quat.LookAt(Vec3.Zero, new Vec3(0,1,0), new Vec3(0,0,-1))));

			// Show the tracking states as text
			Text.Add(c.trackedPos==TrackState.Known?"(pos)":(c.trackedPos==TrackState.Inferred?"~pos~":"pos"), Matrix.TS(0,-0.03f,0, 0.25f));
			Text.Add(c.trackedRot==TrackState.Known?"(rot)":(c.trackedRot==TrackState.Inferred?"~rot~":"rot"), Matrix.TS(0,-0.02f,0, 0.25f));

			// Show the controller's buttons
			Text.Add(Input.ControllerMenuButton.IsActive()?"(menu)":"menu", Matrix.TS(0,-0.01f,0, 0.25f));
			Text.Add(c.IsX1Pressed?"(X1)":"X1", Matrix.TS(0,0.00f,0, 0.25f));
			Text.Add(c.IsX2Pressed?"(X2)":"X2", Matrix.TS(0,0.01f,0, 0.25f));

			// Show the analog stick's information
			Vec3 stickAt = new Vec3(0, 0.03f, 0);
			Lines.Add(stickAt, stickAt + c.stick.XY0*0.01f, Color.White, 0.001f);
			if (c.IsStickClicked) Text.Add("O", Matrix.TS(stickAt, 0.25f));

			// And show the trigger and grip buttons
			Default.MeshCube.Draw(Default.Material, Matrix.TS(0, -0.015f, -0.005f, new Vec3(0.01f, 0.04f, 0.01f)) * Matrix.TR(new Vec3(0,0.02f,0.03f), Quat.FromAngles(-45+c.trigger*40, 0,0) ));
			Default.MeshCube.Draw(Default.Material, Matrix.TS(0.0149f*(hand == Handed.Right?1:-1), 0, 0.015f, new Vec3(0.01f*(1-c.grip), 0.04f, 0.01f)));

		Hierarchy.Pop();
	Hierarchy.Pop();

	// And show the pointer
	Default.MeshCube.Draw(Default.Material, c.aim.ToMatrix(new Vec3(1,1,4) * U.cm), Color.HSV(0,0.5f,0.8f).ToLinear());
}
```

