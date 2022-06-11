---
layout: default
title: SK.AddStepper
description: This creates and registers an instance the IStepper type provided as the generic parameter. SK will hold onto it, Initialize it, Step it every frame, and call Shutdown when the application ends. This is generally safe to do before SK.Initialize is called, the constructor is called right away, and Initialize is called right after SK.Initialize, or right away if SK is already initialized.
---
# [SK]({{site.url}}/Pages/Reference/SK.html).AddStepper

<div class='signature' markdown='1'>
```csharp
static Object AddStepper(Type type)
```
This creates and registers an instance the `IStepper` type
provided as the generic parameter. SK will hold onto it, Initialize
it, Step it every frame, and call Shutdown when the application
ends. This is generally safe to do before SK.Initialize is called,
the constructor is called right away, and Initialize is called
right after SK.Initialize, or right away if SK is already
initialized.
</div>

|  |  |
|--|--|
|Type type|Any object that implements IStepper, and has a             constructor with zero parameters.|
|RETURNS: Object|Just for convenience, this returns the instance that was just added.|




