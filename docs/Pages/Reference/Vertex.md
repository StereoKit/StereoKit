---
layout: default
title: Vertex
description: This represents a single vertex in a Mesh, all StereoKit Meshes currently use this exact layout!
---
# struct Vertex

This represents a single vertex in a Mesh, all StereoKit
Meshes currently use this exact layout!


## Instance Fields and Properties

|  |  |
|--|--|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) [col]({{site.url}}/Pages/Reference/Vertex/col.html)|The color of the vertex. If you aren't using it, set it to white.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [norm]({{site.url}}/Pages/Reference/Vertex/norm.html)|The normal of this vertex, or the direction the vertex is facing. Preferably normalized.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [pos]({{site.url}}/Pages/Reference/Vertex/pos.html)|Position of the vertex, in model space coordinates.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [uv]({{site.url}}/Pages/Reference/Vertex/uv.html)|The texture coordinates at this vertex.|




