---
layout: default
title: UIMove
description: This describes how a UI element moves when being dragged around by a user!
---
# enum UIMove

This describes how a UI element moves when being dragged
around by a user!

## Enum Values

|  |  |
|--|--|
|Exact|The element follows the position and orientation of the user's hand exactly.|
|FaceUser|The element follows the position of the user's hand, but orients to face the user's head instead of just using the hand's rotation.|
|None|Do not allow user input to change the element's pose at all! You may also be interested in UI.Push/PopSurface.|
|PosOnly|This element follows the hand's position only, completely discarding any rotation information.|
