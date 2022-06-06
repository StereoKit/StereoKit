---
layout: default
title: BtnState
description: A bit-flag for the current state of a button input.
---
# enum BtnState

A bit-flag for the current state of a button input.

## Enum Values

|  |  |
|--|--|
|Active|Is the button currently down, pressed?|
|Any|Matches with all states!|
|Changed|Has the button just changed state this frame?|
|Inactive|Is the button currently up, unpressed?|
|JustActive|Has the button just been pressed? Only true for a single frame.|
|JustInactive|Has the button just been released? Only true for a single frame.|

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

