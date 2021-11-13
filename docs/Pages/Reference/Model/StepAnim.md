---
layout: default
title: Model.StepAnim
description: Calling Draw will automatically step the Model's animation, but if you don't draw the Model, or need access to the animated nodes before drawing, then you can step the animation early manually via this method. Animation will only ever be stepped once per frame, so it's okay to call this multiple times, or in addition to Draw.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).StepAnim

<div class='signature' markdown='1'>
```csharp
void StepAnim()
```
Calling Draw will automatically step the Model's
animation, but if you don't draw the Model, or need access to the
animated nodes before drawing, then you can step the animation
early manually via this method. Animation will only ever be stepped
once per frame, so it's okay to call this multiple times, or in
addition to Draw.
</div>




