---
layout: default
title: AppFocus
description: This tells about the app's current focus state, whether it's active and receiving input, or if it's backgrounded or hidden. This can be important since apps may still run and render when unfocused, as the app may still be visible behind the app that _does_ have focus.
---
# enum AppFocus

This tells about the app's current focus state, whether it's active and
receiving input, or if it's backgrounded or hidden. This can be
important since apps may still run and render when unfocused, as the app
may still be visible behind the app that _does_ have focus.

## Enum Values

|  |  |
|--|--|
|Active|This StereoKit app is active, focused, and receiving input from the user. Application should behave as normal.|
|Background|This StereoKit app has been unfocused, something may be compositing on top of the app such as an OS dashboard. The app is still visible, but some other thing has focus and is receiving input. You may wish to pause, disable input tracking, or other such things.|
|Hidden|This app is not rendering currently.|

## Examples

### Checking for changes in application focus
```csharp
AppFocus lastFocus = AppFocus.Hidden;
void CheckFocus()
{
	if (lastFocus != SK.AppFocus)
	{
		lastFocus = SK.AppFocus;
		Log.Info($"App focus changed to: {lastFocus}");
	}
}
```

