---
layout: default
title: Guides Using Hands
description: Using Hands
---

Using hands is easy, just do this!
```
if (Input.Hand(Handed.Right).IsJustPinched)
{
    objects.Add(new Solid(new Vec3(0, 3, 0), Quat.Identity));
    objects.Last().AddSphere(.45f, 40);
    objects.Last().AddBox(Vec3.One * .35f, 40);
}
```

