---
layout: default
title: SolidType
description: This describes the behavior of a 'Solid' physics object! The physics engine will apply forces differently based on this type.
---
# enum SolidType

This describes the behavior of a 'Solid' physics object! The
physics engine will apply forces differently based on this type.

## Enum Values

|  |  |
|--|--|
|Immovable|Immovable objects are always stationary! They have infinite mass, zero velocity, and can't collide with Immovable of Unaffected types.|
|Normal|This object behaves like a normal physical object, it'll fall, get pushed around, and generally be susceptible to physical forces! This is a 'Dynamic' body in physics simulation terms.|
|Unaffected|Unaffected objects have infinite mass, but can have a velocity! They'll move under their own forces, but nothing in the simulation will affect them. They don't collide with Immovable or Unaffected types.|
