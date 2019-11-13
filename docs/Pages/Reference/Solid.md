---
layout: default
title: Solid
description: A Solid is an object that gets simulated with physics! Once you create a solid, it will continuously be acted upon by forces like gravity and other objects. Solid does -not- draw anything on its own, but you can ask a Solid for its current pose, and draw an object at that pose!  Once you create a Solid, you need to define its shape using geometric primitives, this is the AddSphere, AddCube, AddCapsule functions. You can add more than one to a single Solid to get a more complex shape!  If you want to directly move a Solid, note the difference between the Move function and the Teleport function. Move will change the velocity for a single frame to travel through space to get to its destination, while teleport will simply appear at its destination without touching anything between.
---
# Solid

A Solid is an object that gets simulated with physics! Once you
create a solid, it will continuously be acted upon by forces like gravity
and other objects. Solid does -not- draw anything on its own, but you can
ask a Solid for its current pose, and draw an object at that pose!

Once you create a Solid, you need to define its shape using geometric
primitives, this is the AddSphere, AddCube, AddCapsule functions. You can
add more than one to a single Solid to get a more complex shape!

If you want to directly move a Solid, note the difference between the Move
function and the Teleport function. Move will change the velocity for a
single frame to travel through space to get to its destination, while teleport
will simply appear at its destination without touching anything between.



## Methods

|  |  |
|--|--|
|[Solid]({{site.url}}/Pages/Reference/Solid/Solid.html)|Creates a Solid physics object and adds it to the physics system.|


