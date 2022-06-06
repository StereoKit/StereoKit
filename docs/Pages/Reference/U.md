---
layout: default
title: U
description: A shorthand class with unit multipliers. Helps make code a little more terse on occasions!.
---
# static class U

A shorthand class with unit multipliers. Helps make code a
little more terse on occasions!.

## Static Fields and Properties

|  |  |
|--|--|
|float [cm]({{site.url}}/Pages/Reference/U/cm.html)|Converts centimeters to meters. There are 100cm in 1m. In StereoKit 1 unit is also 1 meter, so `25 * U.cm == 0.25`, 25 centimeters is .25 meters/units.|
|float [km]({{site.url}}/Pages/Reference/U/km.html)|Converts meters to kilometers. There are 1000m in 1km, so this just multiplies by 1000.|
|float [m]({{site.url}}/Pages/Reference/U/m.html)|StereoKit's default unit is meters, but sometimes it's nice to be explicit!|
|float [mm]({{site.url}}/Pages/Reference/U/mm.html)|Converts millimeters to meters. There are 1000mm in 1m. In StereoKit 1 unit is 1 meter, so `250 * Units.mm2m == 0.25`, 250 millimeters is .25 meters/units.|
