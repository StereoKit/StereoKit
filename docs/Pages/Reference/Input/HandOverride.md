---
layout: default
title: Input.HandOverride
description: This allows you to completely override the hand's pose information! It is still treated like the user's hand, so this is great for simulating input for testing purposes. It will remain overridden until you call Input.HandClearOverride.
---
# [Input]({{site.url}}/Pages/Reference/Input.html).HandOverride

<div class='signature' markdown='1'>
```csharp
static void HandOverride(Handed hand, HandJoint[]& joints)
```
This allows you to completely override the hand's pose
information! It is still treated like the user's hand, so this is
great for simulating input for testing purposes. It will remain
overridden until you call Input.HandClearOverride.
</div>

|  |  |
|--|--|
|[Handed]({{site.url}}/Pages/Reference/Handed.html) hand|Which hand should be overridden?|
|HandJoint[]& joints|A 2D array of 25 joints that should be used             as StereoKit's hand information. See `Hand.fingers` for more              information.|




