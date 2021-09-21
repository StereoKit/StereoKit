---
layout: default
title: ModelNode.ModelTransform
description: The transform of this node relative to the Model itself. This incorporates transforms from all parent nodes. Setting this transform will update the LocalTransform, as well as all Child nodes below this one.
---
# [ModelNode]({{site.url}}/Pages/Reference/ModelNode.html).ModelTransform

<div class='signature' markdown='1'>
[Matrix]({{site.url}}/Pages/Reference/Matrix.html) ModelTransform{ get set }
</div>

## Description
The transform of this node relative to the Model itself.
This incorporates transforms from all parent nodes. Setting this
transform will update the LocalTransform, as well as all Child
nodes below this one.

