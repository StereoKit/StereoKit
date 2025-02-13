/// :CodeDoc: Guides 7 Working with 3D Assets
/// # Working with 3D Assets
///
/// StereoKit's core 3D asset format is the GLTF! While there is still support
/// for other formats, like STL, OBJ and PLY, GLTF is StereoKit's preferred
/// format of choice. It has a well defined modern specification, and a large
/// collection of quality tooling available in the ecosystem.
///
/// > GLB is still just the "binary" format of GLTF, typically with all related
/// > textures bundled inside it.
///
/// I've found [Blender](https://www.blender.org/) to have a very nice GLTF
/// exporter! So if your tool of choice doesn't support GLTF yet, exporting to
/// Blender for a final pass can be a good approach! However, most tools will at
/// least have a plugin for GLTF export.
///
/// Most 3D asset creation tools have material systems more geared towards
/// cinematic rendering, rather than realtime rendering, so it can be important
/// to understand how materials are exported in GLTF format [(via Blender)](https://docs.blender.org/manual/en/2.80/addons/io_scene_gltf2.html#exported-materials),
/// as well as how StereoKit interprets them.
///
/// ## Materials
///
/// StereoKit's rules for interpreting GLTF materials are mostly
/// straightforward!
///
/// If the GLTF material is PBR (the normal case), StereoKit will use a PBR
/// shader. It uses the metallic roughness definition of PBR, and does not
/// currently support the specular/glossiness GLTF extension.
///
/// If the GLTF material is Unlit (In Blender, this is a material using a
/// Background or Emission surface) StereoKit will use an Unlit shader.
///
/// If the GLTF material has alpha mask on, StereoKit will use a "clip" variant
/// of the PBR or Unlit shaders that discards transparent pixels.
///
/// > It's good to note that GLTF supports vertex color data, and almost all
/// > StereoKit default shaders support vertex coloring! Vertex colors don't
/// > often show in 3D asset tools out-of-the-box, but this can be a nice way to
/// > add cheap baked lighting to Unlit materials, or add color variation to
/// > looping textures, or other creative uses!
///
/// ### Lightmaps
///
/// StereoKit also supports a Ligtmapped material from GLTF files! This is a
/// somewhat non-standard setup not found in the original spec, but is a _very_
/// useful one for performant applications. It is not unusual to find bad GLTFs
/// where lighting data is baked down into the diffuse texture, expanding a
/// "looped" texture into something much much larger. This consumes much more
/// RAM than preserving the original looped texture and combining it with a
/// separate lightmap texture!
/// 
/// Instead, if StereoKit encounters a material that has:
/// - An emissive texture (the base color, like an Unlit material)
/// - An occlusion texture using the second UV channel (the lightmap)
/// - No texture for PBR base color
/// - No texture for PBR metal/roughness
///
/// This will trigger StereoKit to switch to a high-performance Unlit + Lightmap
/// shader.
///
/// ## Best Practices
///
/// ### Optimize with GLTFPack
///
/// StereoKit supports all the GLTF extensions required for [GLTFPack](https://github.com/zeux/meshoptimizer/blob/master/gltf/README.md)
/// to work properly! Mesh compression, KTX textures, quantization and transform
/// all work. If you can optimize your GLTF files in advance with a tool like
/// this, you can get 50-80% smaller files (even on GPU!), much faster load
/// times, and improved render performance!
///
/// ```
/// gltfpack.exe -cc -tc -i modelName.glb -o modelName.opt.glb
/// ```
///
/// This is effective with just about any model, but especially for
/// photogrammetry-like assets this type of optimization is absolutely critical!
///
/// ### Backface Culling
///
/// GLTF supports a setting for enabling or disabling backface culling, an
/// important optimization that will skip rendering triangles that face away
/// from the viewer. Having backface culling ON can improve performance
/// significantly, and in most normal cases, is completely unnoticeable! For
/// whatever reason, many 3D tools seem to disable backface culling by default,
/// and this mistake is very common to find in many GLTF files!
///
/// Always make sure your materials have backface culling ON whenever possible!
///
/// ![Settings for turning on backface culling in Blender](/img/3DAssetsBackfaceCulling.png)
///
/// ### PBR Shaders vs. Unlit
///
/// Physically Based shaders look great! But the accuracy they provide costs a
/// lot of computation. If you can use an Unlit material instead, do it!
///
/// For more information about shaders and material, check out StereoKit's
/// [Material guide]({{site.url}}/Pages/Guides/Working-with-Materials.html)!
/// 
/// ### References
///
/// A quick note about how Asset types (Model/Material/Tex, etc.) work in
/// StereoKit!
///
/// When you work with an Asset, you'll want to keep in mind that you're
/// actually working with a reference to the Asset! Sometimes the handle you
/// have is just one of _many_ references to the Asset, and so changing the
/// Asset will affect all other references to it as well.
///
/// If you want to change an Asset's property _without_ affecting other
/// references to that Asset, then you should `Copy()` that Asset, and modify
/// that new, unique instance instead! This is why you'll frequently see the
/// example code doing something like `Material.Default.Copy()` before
/// changing any of the properties.
///
/// ## GLTF extension support
///
/// On a more technical note, here's a specific list of what GLTF extensions
/// StereoKit supports.
///
/// - KHR_materials_unlit
/// - KHR_mesh_quantization
/// - KHR_texture_basisu
/// - KHR_texture_transform (not rotation)
/// - EXT_meshopt_compression
///
/// ## Notes About Alternative Formats
///
/// Why no FBX you might ask? FBX is an old and poorly defined format. Early
/// versions of StereoKit supported it, but it ended up being far too much pain.
/// Among other things, FBX has no reliable concept of "units" or even "which
/// direction is forward".
///
/// Why not USD? USD unfortunately has no format specification! It's less of a
/// file format, and more of a library. As such, it has a very poor tooling
/// ecosystem making the "format" difficult to support well. If you want to
/// support USD correctly, you must use the singular implementation of USD.
/// And unfortunately, that implementation doesn't pass my smell test for code
/// quality.
/// :End: