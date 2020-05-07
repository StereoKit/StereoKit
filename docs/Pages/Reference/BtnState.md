---
layout: default
title: BtnState
description: A bit-flag for the current state of a button input.
---
# BtnState

A bit-flag for the current state of a button input.




## Static Fields and Properties

|  |  |
|--|--|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [Active]({{site.url}}/Pages/Reference/BtnState/Active.html)|Is the button currently down, pressed?|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [Any]({{site.url}}/Pages/Reference/BtnState/Any.html)|Matches with all states!|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [Changed]({{site.url}}/Pages/Reference/BtnState/Changed.html)|Has the button just changed state this frame?|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [Inactive]({{site.url}}/Pages/Reference/BtnState/Inactive.html)|Is the button currently up, unpressed?|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [JustActive]({{site.url}}/Pages/Reference/BtnState/JustActive.html)|Has the button just been pressed? Only true for a single frame.|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [JustInactive]({{site.url}}/Pages/Reference/BtnState/JustInactive.html)|Has the button just been released? Only true for a single frame.|



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

