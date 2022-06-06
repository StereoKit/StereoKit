---
layout: default
title: Solid
description: A Solid is an object that gets simulated with physics! Once you create a solid, it will continuously be acted upon by forces like gravity and other objects. Solid does -not- draw anything on its own, but you can ask a Solid for its current pose, and draw an object at that pose!  Once you create a Solid, you need to define its shape using geometric primitives, this is the AddSphere, AddCube, AddCapsule functions. You can add more than one to a single Solid to get a more complex shape!  If you want to directly move a Solid, note the difference between the Move function and the Teleport function. Move will change the velocity for a single frame to travel through space to get to its destination, while teleport will simply appear at its destination without touching anything between.
---
# class Solid

A Solid is an object that gets simulated with physics! Once
you create a solid, it will continuously be acted upon by forces like
gravity and other objects. Solid does -not- draw anything on its own,
but you can ask a Solid for its current pose, and draw an object at
that pose!

Once you create a Solid, you need to define its shape using geometric
primitives, this is the AddSphere, AddCube, AddCapsule functions. You
can add more than one to a single Solid to get a more complex shape!

If you want to directly move a Solid, note the difference between the
Move function and the Teleport function. Move will change the
velocity for a single frame to travel through space to get to its
destination, while teleport will simply appear at its destination
without touching anything between.

## Instance Fields and Properties

|  |  |
|--|--|
|bool [Enabled]({{site.url}}/Pages/Reference/Solid/Enabled.html)|Is the Solid enabled in the physics simulation? Set this to false if you want to prevent physics from influencing this solid!|

## Instance Methods

|  |  |
|--|--|
|[Solid]({{site.url}}/Pages/Reference/Solid/Solid.html)|Creates a Solid physics object and adds it to the physics system.|
|[AddBox]({{site.url}}/Pages/Reference/Solid/AddBox.html)|This adds a box to this solid's physical shape! This is in addition to any other shapes you've added to this solid.|
|[AddCapsule]({{site.url}}/Pages/Reference/Solid/AddCapsule.html)|This adds a capsule, a cylinder with rounded ends, to this solid's physical shape! This is in addition to any other shapes you've added to this solid.|
|[AddSphere]({{site.url}}/Pages/Reference/Solid/AddSphere.html)|This adds a sphere to this solid's physical shape! This is in addition to any other shapes you've added to this solid.|
|[GetPose]({{site.url}}/Pages/Reference/Solid/GetPose.html)|Retrieves the current pose of the Solid from the physics simulation.|
|[Move]({{site.url}}/Pages/Reference/Solid/Move.html)|This moves the Solid from its current location through space to the new location provided, colliding with things along the way. This is achieved by applying the velocity and angular velocity necessary to get to the destination in a single frame during the next physics step, then restoring the previous velocity info afterwards! See also Teleport for movement without collision.|
|[SetAngularVelocity]({{site.url}}/Pages/Reference/Solid/SetAngularVelocity.html)|I wish I knew what angular velocity actually meant, perhaps you know? It's rotation velocity of some kind or another!|
|[SetEnabled]({{site.url}}/Pages/Reference/Solid/SetEnabled.html)|Same as Enabled. Is the Solid enabled in the physics simulation? Set this to false if you want to prevent physics from influencing this solid!|
|[SetType]({{site.url}}/Pages/Reference/Solid/SetType.html)|Changes the behavior type of the solid after it's created.|
|[SetVelocity]({{site.url}}/Pages/Reference/Solid/SetVelocity.html)|Sets the velocity of this Solid.|
|[Teleport]({{site.url}}/Pages/Reference/Solid/Teleport.html)|Moves the Solid to the new pose, without colliding with objects on the way there.|
