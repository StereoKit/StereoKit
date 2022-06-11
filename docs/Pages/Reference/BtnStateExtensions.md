---
layout: default
title: BtnStateExtensions
description: A collection of extension methods for the BtnState enum that makes bit-field checks a little easier.
---
# static class BtnStateExtensions

A collection of extension methods for the BtnState enum that makes
bit-field checks a little easier.

## Static Methods

|  |  |
|--|--|
|[IsActive]({{site.url}}/Pages/Reference/BtnStateExtensions/IsActive.html)|Is the button pressed?|
|[IsChanged]({{site.url}}/Pages/Reference/BtnStateExtensions/IsChanged.html)|Was the button either presses or released this frame?|
|[IsJustActive]({{site.url}}/Pages/Reference/BtnStateExtensions/IsJustActive.html)|Has the button just been pressed this frame?|
|[IsJustInactive]({{site.url}}/Pages/Reference/BtnStateExtensions/IsJustInactive.html)|Has the button just been released this frame?|

## Examples

```csharp
BtnState state = Input.Hand(Handed.Right).pinch;

// You can check a BtnState using bit-flag logic
if ((state & BtnState.Changed) > 0)
	Log.Info("Pinch state just changed!");

// Or you can check the same values with the extension methods, no
// bit flag logic :)
if (state.IsChanged())
	Log.Info("Pinch state just changed!");
```

