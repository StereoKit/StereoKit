---
layout: default
title: Input.HandVisible
description: Sets whether or not StereoKit should render the hand for you. Turn this to false if you're going to render your own, or don't need the hand itself to be visible.
---
# [Input]({{site.url}}/Pages/Reference/Input.html).HandVisible

<div class='signature' markdown='1'>
```csharp
static void HandVisible(Handed hand, bool visible)
```
Sets whether or not StereoKit should render the hand for
you. Turn this to false if you're going to render your own, or
don't need the hand itself to be visible.
</div>

|  |  |
|--|--|
|[Handed]({{site.url}}/Pages/Reference/Handed.html) hand|If Handed.Max, this will set the value for              both hands.|
|bool visible|True, StereoKit renders this. False, it             doesn't.|




