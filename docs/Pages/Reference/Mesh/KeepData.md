---
layout: default
title: Mesh.KeepData
description: Should StereoKit keep the mesh data on the CPU for later access, or collision detection? Defaults to true. If you set this to false before setting data, the data won't be stored. If you call this after setting data, that stored data will be freed! If you set this to true again later on, it will not contain data until it's set again.
---
# [Mesh]({{site.url}}/Pages/Reference/Mesh.html).KeepData

<div class='signature' markdown='1'>
bool KeepData{ get set }
</div>

## Description
Should StereoKit keep the mesh data on the CPU for later
access, or collision detection? Defaults to true. If you set this
to false before setting data, the data won't be stored. If you
call this after setting data, that stored data will be freed! If
you set this to true again later on, it will not contain data
until it's set again.

