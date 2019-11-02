---
layout: default
title: Units.mm2m
description: Converts millimeters to meters. There are 1000mm in 1m. In StereoKit              1 unit is 1 meter, so 250 * Units.mm2m == 0.25, 250 millimeters is .25              meters/units.
---
# [Units]({{url}}pages/Reference/Units.html).mm2m

## Description
Converts millimeters to meters. There are 1000mm in 1m. In StereoKit 
            1 unit is 1 meter, so `250 * Units.mm2m == 0.25`, 250 millimeters is .25 
            meters/units.


## Examples

Here, we're doing a window!
With lots of explanation.
and lines
```
UI.WindowBegin("Main", ref window_pose, new Vec2(24, 0) * Units.cm2m);

```
Maybe we'll break it up into sections?
```
// Regular comment
UI.Button("Testing!\nok"); UI.SameLine();
UI.Button("Another");
UI.HSlider("slider",  ref slider1, 0, 1, 0.2f, 72 * Units.mm2m); UI.SameLine();
UI.HSlider("slider2", ref slider2, 0, 1, 0,    72 * Units.mm2m);
if (UI.Button("Press me!"))
    UI.Button("DYANMIC BUTTON!!");

UI.WindowEnd();
```
That's all folks!
Whaddya think?

