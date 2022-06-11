---
layout: default
title: SK.AppFocus
description: This tells about the app's current focus state, whether it's active and receiving input, or if it's backgrounded or hidden. This can be important since apps may still run and render when unfocused, as the app may still be visible behind the app that _does_ have focus.
---
# [SK]({{site.url}}/Pages/Reference/SK.html).AppFocus

<div class='signature' markdown='1'>
static [AppFocus]({{site.url}}/Pages/Reference/AppFocus.html) AppFocus{ get }
</div>

## Description
This tells about the app's current focus state,
whether it's active and receiving input, or if it's
backgrounded or hidden. This can be important since apps may
still run and render when unfocused, as the app may still be
visible behind the app that _does_ have focus.


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

