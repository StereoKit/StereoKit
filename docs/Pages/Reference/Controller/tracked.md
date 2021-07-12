---
layout: default
title: Controller.tracked
description: This tells the current tracking state of this controller overall. If either position or rotation are trackable, then this will report tracked. Typically, positional tracking will be lost first, when the controller goes out of view, and rotational tracking will often remain as long as the controller is still connected. This is a good way to check if the controller is connected to the system at all.
---
# [Controller]({{site.url}}/Pages/Reference/Controller.html).tracked

<div class='signature' markdown='1'>
[BtnState]({{site.url}}/Pages/Reference/BtnState.html) tracked
</div>

## Description
This tells the current tracking state of this controller
overall. If either position or rotation are trackable, then this
will report tracked. Typically, positional tracking will be lost
first, when the controller goes out of view, and rotational
tracking will often remain as long as the controller is still
connected. This is a good way to check if the controller is
connected to the system at all.

