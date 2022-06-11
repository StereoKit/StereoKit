---
layout: default
title: UIVisual
description: Used with StereoKit's UI to indicate a particular type of UI element visual.
---
# enum UIVisual

Used with StereoKit's UI to indicate a particular type of UI
element visual.

## Enum Values

|  |  |
|--|--|
|Button|Refers to UI.Button elements.|
|Default|A default root UI element. Not a particular element, but other elements may refer to this if there is nothing more specific present.|
|Handle|Refers to UI.Handle/HandleBegin elements.|
|Input|Refers to UI.Input elements.|
|Max|A maximum enum value to allow for iterating through enum values.|
|None|Default state, no UI element at all.|
|Separator|Refers to UI.HSeparator element.|
|SliderLine|Refers to the back line component of the UI.HSlider element.|
|SliderPinch|Refers to the pinch button component of the UI.HSlider element when using UIConfirm.Pinch.|
|SliderPush|Refers to the push button component of the UI.HSlider element when using UIConfirm.Push.|
|Toggle|Refers to UI.Toggle elements.|
|WindowBody|Refers to UI.Window/WindowBegin body panel element, this element is used when a Window head is also present.|
|WindowBodyOnly|Refers to UI.Window/WindowBegin body element, this element is used when a Window only has the body panel, without a head.|
|WindowHead|Refers to UI.Window/WindowBegin head panel element, this element is used when a Window body is also present.|
|WindowHeadOnly|Refers to UI.Window/WindowBegin head element, this element is used when a Window only has the head panel, without a body.|
