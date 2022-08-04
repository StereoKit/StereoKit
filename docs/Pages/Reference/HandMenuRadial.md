---
layout: default
title: HandMenuRadial
description: A menu that shows up in circle around the user's hand! Selecting an item can perform an action, or even spawn a sub-layer of menu items. This is an easy way to store actions out of the way, yet remain easily accessible to the user.  The user faces their palm towards their head, and then makes a grip motion to spawn the menu. The user can then perform actions by making fast, direction based motions that are easy to build muscle memory for.
---
# class HandMenuRadial

A menu that shows up in circle around the user's
hand! Selecting an item can perform an action, or even spawn
a sub-layer of menu items. This is an easy way to store
actions out of the way, yet remain easily accessible to the
user.

The user faces their palm towards their head, and then makes
a grip motion to spawn the menu. The user can then perform actions
by making fast, direction based motions that are easy to build
muscle memory for.

## Instance Fields and Properties

|  |  |
|--|--|
|bool [Enabled]({{site.url}}/Pages/Reference/HandMenuRadial/Enabled.html)|HandMenuRadial is always Enabled.|
|[HandRadialLayer]({{site.url}}/Pages/Reference/HandRadialLayer.html) [RootLayer]({{site.url}}/Pages/Reference/HandMenuRadial/RootLayer.html)|This returns the root menu layer, this is always the first layer added to the list of layers. There should always be a root layer, and if the HandMenuRadial is created without any layers, a root layer will be added automatically.|

## Instance Methods

|  |  |
|--|--|
|[HandMenuRadial]({{site.url}}/Pages/Reference/HandMenuRadial/HandMenuRadial.html)|Creates a hand menu from the provided array of menu layers! HandMenuRadial is an IStepper, so proper usage is to add it to the Stepper list via `StereoKitApp.AddStepper`. If no layers are provided to this constructor, a default root layer will be automatically added.|
|[Close]({{site.url}}/Pages/Reference/HandMenuRadial/Close.html)|Closes the menu if it's open! Plays a closing sound.|
|[Initialize]({{site.url}}/Pages/Reference/HandMenuRadial/Initialize.html)|Part of IStepper, you shouldn't be calling this yourself.|
|[Show]({{site.url}}/Pages/Reference/HandMenuRadial/Show.html)|Force the hand menu to show at a specific location. This will close the hand menu if it was already open, and resets it to the root menu layer. Also plays an opening sound.|
|[Shutdown]({{site.url}}/Pages/Reference/HandMenuRadial/Shutdown.html)|Part of IStepper, you shouldn't be calling this yourself.|
|[Step]({{site.url}}/Pages/Reference/HandMenuRadial/Step.html)|Part of IStepper, you shouldn't be calling this yourself.|

## Static Fields and Properties

|  |  |
|--|--|
|[Key]({{site.url}}/Pages/Reference/Key.html) [simulatorKey]({{site.url}}/Pages/Reference/HandMenuRadial/simulatorKey.html)|When using the Simulator, this key will activate the menu on the current hand, regardless of which direction it is facing.|

## Examples

### Basic layered hand menu

The HandMenuRadial is an `IStepper`, so it should be registered with
`StereoKitApp.AddStepper` so it can run by itself! It's recommended to
keep track of it anyway, so you can remove it when you're done with it
via `StereoKitApp.RemoveStepper`

The constructor uses a params style argument list that makes it easy and
clean to provide lists of items! This means you can assemble the whole
menu on a single 'line'. You can still pass arrays instead if you prefer
that!
```csharp
handMenu = SK.AddStepper(new HandMenuRadial(
	new HandRadialLayer("Root",
		new HandMenuItem("File",   null, null, "File"),
		new HandMenuItem("Search", null, null, "Edit"),
		new HandMenuItem("About",  Sprite.FromFile("search.png"), () => Log.Info(SK.VersionName)),
		new HandMenuItem("Cancel", null, null)),
	new HandRadialLayer("File", 
		new HandMenuItem("New",   null, () => Log.Info("New")),
		new HandMenuItem("Open",  null, () => Log.Info("Open")),
		new HandMenuItem("Close", null, () => Log.Info("Close")),
		new HandMenuItem("Back",  null, null, HandMenuAction.Back)),
	new HandRadialLayer("Edit",
		new HandMenuItem("Copy",  null, () => Log.Info("Copy")),
		new HandMenuItem("Paste", null, () => Log.Info("Paste")),
		new HandMenuItem("Back", null, null, HandMenuAction.Back))));
```

```csharp
SK.RemoveStepper(handMenu); 
```

