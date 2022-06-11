---
layout: default
title: Units
description: A collection of unit conversion constants! Multiply things by these to convert them into different units.
---
# static class Units

A collection of unit conversion constants! Multiply things by these
to convert them into different units.

## Static Fields and Properties

|  |  |
|--|--|
|float [cm2m]({{site.url}}/Pages/Reference/Units/cm2m.html)|Converts centimeters to meters. There are 100cm in 1m. In StereoKit 1 unit is also 1 meter, so `25 * Units.cm2m == 0.25`, 25 centimeters is .25 meters/units.|
|float [deg2rad]({{site.url}}/Pages/Reference/Units/deg2rad.html)|Degrees to radians, multiply degree values by this, and you get radians! Like so: `180 * Units.deg2rad == 3.1415926536`, which is pi, or 1 radian.|
|float [m2cm]({{site.url}}/Pages/Reference/Units/m2cm.html)|Converts meters to centimeters. There are 100cm in 1m, so this just multiplies by 100.|
|float [m2mm]({{site.url}}/Pages/Reference/Units/m2mm.html)|Converts meters to millimeters. There are 1000mm in 1m, so this just multiplies by 1000.|
|float [mm2m]({{site.url}}/Pages/Reference/Units/mm2m.html)|Converts millimeters to meters. There are 1000mm in 1m. In StereoKit 1 unit is 1 meter, so `250 * Units.mm2m == 0.25`, 250 millimeters is .25 meters/units.|
|float [rad2deg]({{site.url}}/Pages/Reference/Units/rad2deg.html)|Radians to degrees, multiply radian values by this, and you get degrees! Like so: `PI * Units.rad2deg == 180`|
