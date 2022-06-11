---
layout: default
title: SK.RemoveStepper
description: This removes a specific IStepper from SK's IStepper list. This will call the IStepper's Shutdown method before returning.
---
# [SK]({{site.url}}/Pages/Reference/SK.html).RemoveStepper

<div class='signature' markdown='1'>
```csharp
static void RemoveStepper(IStepper stepper)
```
This removes a specific IStepper from SK's IStepper list.
This will call the IStepper's Shutdown method before returning.
</div>

|  |  |
|--|--|
|IStepper stepper|The specific IStepper instance to remove.|

<div class='signature' markdown='1'>
```csharp
static void RemoveStepper(Type type)
```
This removes all IStepper instances that are assignable to
the generic type specified. This will call the IStepper's Shutdown
method on each removed instance before returning.
</div>

|  |  |
|--|--|
|Type type|Any type.|




