---
layout: default
title: Hierarchy
description: This class represents a stack of transform matrices that build up a transform hierarchy! This can be used like an object-less parent-child system, where you push a parent's transform onto the stack, render child objects relative to that parent transform and then pop it off the stack.  Performance note. if any matrices are on the hierarchy stack, any render will cause a matrix multiplication to occur! So if you have a collection of objects with their transforms baked and cached into matrices for performance reasons, you'll want to ensure there are no matrices in the hierarchy stack, or that the hierarchy is disabled! It'll save you a matrix multiplication in that case .)
---
# static class Hierarchy

This class represents a stack of transform matrices that
build up a transform hierarchy! This can be used like an object-less
parent-child system, where you push a parent's transform onto the
stack, render child objects relative to that parent transform and
then pop it off the stack.

Performance note: if any matrices are on the hierarchy stack, any
render will cause a matrix multiplication to occur! So if you have a
collection of objects with their transforms baked and cached into
matrices for performance reasons, you'll want to ensure there are no
matrices in the hierarchy stack, or that the hierarchy is disabled!
It'll save you a matrix multiplication in that case :)

## Static Fields and Properties

|  |  |
|--|--|
|bool [Enabled]({{site.url}}/Pages/Reference/Hierarchy/Enabled.html)|This is enabled by default. Disabling this will cause any draw call to ignore any Matrices that are on the Hierarchy stack.|

## Static Methods

|  |  |
|--|--|
|[Pop]({{site.url}}/Pages/Reference/Hierarchy/Pop.html)|Removes the top Matrix from the stack!|
|[Push]({{site.url}}/Pages/Reference/Hierarchy/Push.html)|Pushes a transform Matrix onto the stack, and combines it with the Matrix below it. Any draw operation's Matrix will now be combined with this Matrix to make it relative to the current hierarchy. Use Hierarchy.Pop to remove it from the Hierarchy stack! All Push calls must have an accompanying Pop call.|
|[ToLocal]({{site.url}}/Pages/Reference/Hierarchy/ToLocal.html)|Converts a world space point into the local space of the current Hierarchy stack!|
|[ToLocalDirection]({{site.url}}/Pages/Reference/Hierarchy/ToLocalDirection.html)|Converts a world space direction into the local space of the current Hierarchy stack! This excludes the translation component normally applied to vectors, so it's still a valid direction.|
|[ToWorld]({{site.url}}/Pages/Reference/Hierarchy/ToWorld.html)|Converts a local point relative to the current hierarchy stack into world space!|
|[ToWorldDirection]({{site.url}}/Pages/Reference/Hierarchy/ToWorldDirection.html)|Converts a local direction relative to the current hierarchy stack into world space! This excludes the translation component normally applied to vectors, so it's still a valid direction.|
