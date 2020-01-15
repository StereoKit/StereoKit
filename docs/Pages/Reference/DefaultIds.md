---
layout: default
title: DefaultIds
description: The default StereoKit assets all have ids, and this class is a collection of all of them in case you want to copy or use them!
---
# DefaultIds

The default StereoKit assets all have ids, and this class is a collection
of all of them in case you want to copy or use them!




## Static Fields and Properties

|  |  |
|--|--|
|string [cubemap]({{site.url}}/Pages/Reference/DefaultIds/cubemap.html)|The default cubemap that StereoKit generates, this is the cubemap that's visible as the background and initial scene lighting.|
|string [font]({{site.url}}/Pages/Reference/DefaultIds/font.html)|The default font asset used by the UI and text. This is currently Segoe!|
|string [material]({{site.url}}/Pages/Reference/DefaultIds/material.html)|The default material! This is used by any model or mesh rendered from within StereoKit. Its shader may change based on system performance characteristics, so it can be great to copy this one when creating your own materials!|
|string [materialEquirect]({{site.url}}/Pages/Reference/DefaultIds/materialEquirect.html)|This material is used for projecting equirectangular textures into cubemap faces. It's probably not a great idea to change this one much!|
|string [materialFont]({{site.url}}/Pages/Reference/DefaultIds/materialFont.html)|This material is used as the default for rendering text! By default, it uses the 'default/shader_font' shader, which is a two-sided alpha-clip shader. This also turns off backface culling.|
|string [quad]({{site.url}}/Pages/Reference/DefaultIds/quad.html)|A default quad mesh, 2 triangles, 4 verts, from (-1,-1,0) to (1,1,0) and facing forward on the Z axis (0,0,-1). White vertex colors, and UVs from (0,0) at vertex (-1,-1,0) to (1,1) at vertex (1,1,0).|
|string [shader]({{site.url}}/Pages/Reference/DefaultIds/shader.html)|This is a fast, general purpose shader. It uses a texture for 'diffuse', a 'color' property for tinting the material, and a 'tex_scale' for scaling the UV coordinates. For lighting, it just uses a lookup from the current cubemap.|
|string [shaderEquirect]({{site.url}}/Pages/Reference/DefaultIds/shaderEquirect.html)|A shader for projecting equirectangular textures onto cube faces! This is for equirectangular texture loading.|
|string [shaderFont]({{site.url}}/Pages/Reference/DefaultIds/shaderFont.html)|A shader for text! Right now, this will render a font atlas texture, and perform alpha-testing for transparency. It also flips normals of the back-face of the surface, so backfaces get lit properly when backface culling is turned off, as it is by default for text.|
|string [shaderPbr]({{site.url}}/Pages/Reference/DefaultIds/shaderPbr.html)|A physically based shader.|
|string [shaderUnlit]({{site.url}}/Pages/Reference/DefaultIds/shaderUnlit.html)|Sometimes lighting just gets in the way! This is an extremely simple and fast shader that uses a 'diffuse' texture and a 'color' tint property to draw a model without any lighting at all!|
|string [tex]({{site.url}}/Pages/Reference/DefaultIds/tex.html)|Default 2x2 white opaque texture, this is the texture referred to as 'white' in the shader texture defaults.|
|string [texBlack]({{site.url}}/Pages/Reference/DefaultIds/texBlack.html)|Default 2x2 black opaque texture, this is the texture referred to as 'black' in the shader texture defaults.|
|string [texFlat]({{site.url}}/Pages/Reference/DefaultIds/texFlat.html)|Default 2x2 flat normal texture, this is a normal that faces out from the, face, and has a color value of (0.5,0.5,1). this is the texture referred to as 'flat' in the shader texture defaults.|
|string [texGray]({{site.url}}/Pages/Reference/DefaultIds/texGray.html)|Default 2x2 middle gray (0.5,0.5,0.5) opaque texture, this is the texture referred to as 'gray' in the shader texture defaults.|
|string [texRough]({{site.url}}/Pages/Reference/DefaultIds/texRough.html)|Default 2x2 roughness color (0,0,1) texture, this is the texture referred to as 'rough' in the shader texture defaults.|


