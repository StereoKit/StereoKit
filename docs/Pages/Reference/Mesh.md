---
layout: default
title: Mesh
description: A Mesh is a single collection of triangular faces with extra surface information to enhance rendering! StereoKit meshes are composed of a list of vertices, and a list of indices to connect the vertices into faces. Nothing more than that is stored here, so typically meshes are combined with Materials, or added to Models in order to draw them.  Mesh vertices are composed of a position, a normal (direction of the vert), a uv coordinate (for mapping a texture to the mesh's surface), and a 32 bit color containing red, green, blue, and alpha (transparency).  Mesh indices are stored as unsigned shorts, so they cap out at 65,535. This limits meshes to 65,535 vertices. I may change this to integers later, now that I think about it...
---
# Mesh

A Mesh is a single collection of triangular faces with extra surface information to enhance
rendering! StereoKit meshes are composed of a list of vertices, and a list of indices to
connect the vertices into faces. Nothing more than that is stored here, so typically meshes
are combined with Materials, or added to Models in order to draw them.

Mesh vertices are composed of a position, a normal (direction of the vert), a uv coordinate
(for mapping a texture to the mesh's surface), and a 32 bit color containing red, green, blue,
and alpha (transparency).

Mesh indices are stored as unsigned shorts, so they cap out at 65,535. This limits meshes to
65,535 vertices. I may change this to integers later, now that I think about it...



## Methods

|  |  |
|--|--|
|[Mesh]({{site.url}}/Pages/Reference/Mesh/Mesh.html)|Creates an empty Mesh asset. Use SetVerts and SetInds to add data to it!|
|[Find]({{site.url}}/Pages/Reference/Mesh/Find.html)|Finds the Mesh with the matching id, and returns a reference to it. If no Mesh it found, it returns null.|
|[GenerateCube]({{site.url}}/Pages/Reference/Mesh/GenerateCube.html)|Generates a flat-shaded cube mesh, pre-sized to the given dimensions. UV coordinates are projected flat on each face, 0,0 -> 1,1.|
|[GenerateCylinder]({{site.url}}/Pages/Reference/Mesh/GenerateCylinder.html)|Generates a cylinder mesh, pre-sized to the given diameter and depth, UV coordinates are from a flattened top view right now. Additional development is needed for making better UVs for the edges.|
|[GenerateRoundedCube]({{site.url}}/Pages/Reference/Mesh/GenerateRoundedCube.html)|Generates a cube mesh with rounded corners, pre-sized to the given dimensions. UV coordinates are 0,0 -> 1,1 on each face, meeting at the middle of the rounded corners.|
|[GenerateSphere]({{site.url}}/Pages/Reference/Mesh/GenerateSphere.html)|Generates a sphere mesh, pre-sized to the given diameter, created by sphereifying a subdivided cube! UV coordinates are taken from the initial unspherified cube.|


