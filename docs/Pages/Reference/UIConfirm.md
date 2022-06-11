---
layout: default
title: UIConfirm
description: Used with StereoKit's UI, and determines the interaction confirmation behavior for certain elements, such as the UI.HSlider!
---
# enum UIConfirm

Used with StereoKit's UI, and determines the interaction
confirmation behavior for certain elements, such as the UI.HSlider!

## Enum Values

|  |  |
|--|--|
|Pinch|The user must use a pinch gesture to interact with this element. This is much harder to activate by accident, but does require the user to make a precise pinch gesture. You can pretty much be sure that's what the user meant to do!|
|Push|The user must push a button with their finger to confirm interaction with this element. This is simpler to activate as it requires no learned gestures, but may result in more false positives.|
|VariablePinch|HSlider specific. Same as Pinch, but pulling out from the slider creates a scaled slider that lets you adjust the slider at a more granular resolution.|
