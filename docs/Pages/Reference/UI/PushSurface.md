---
layout: default
title: UI.PushSurface
description: This will push a surface into SK's UI layout system. The surface becomes part of the transform hierarchy, and SK creates a layout surface for UI content to be placed on and interacted with. Must be accompanied by a PopSurface call.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).PushSurface

<div class='signature' markdown='1'>
```csharp
static void PushSurface(Pose surfacePose, Vec3 layoutStart, Vec2 layoutDimensions)
```
This will push a surface into SK's UI layout system. The
surface becomes part of the transform hierarchy, and SK creates a
layout surface for UI content to be placed on and interacted with.
Must be accompanied by a PopSurface call.
</div>

|  |  |
|--|--|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) surfacePose|The Pose of the UI surface, where the             surface forward direction is the same as the Pose's.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) layoutStart|This is an offset from the center of the             coordinate space created by the surfacePose. Vec3.Zero would mean             that content starts at the center of the surfacePose.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) layoutDimensions|The size of the surface area to use             during layout. Like other UI layout sizes, an axis set to zero             means it will auto-expand in that direction.|




