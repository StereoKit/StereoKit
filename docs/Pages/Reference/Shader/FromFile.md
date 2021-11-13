---
layout: default
title: Shader.FromFile
description: Loads a shader from a precompiled StereoKit Shader (.sks) file! HLSL files can be compiled using the skshaderc tool included in the NuGet package. This should be taken care of by MsBuild automatically, but you may need to ensure your HLSL file is a <SKShader /> item type in the .csproj for this to work. You can also compile with the command line app manually if you're compiling/distributing a shader some other way!
---
# [Shader]({{site.url}}/Pages/Reference/Shader.html).FromFile

<div class='signature' markdown='1'>
```csharp
static Shader FromFile(string file)
```
Loads a shader from a precompiled StereoKit Shader
(.sks) file! HLSL files can be compiled using the skshaderc tool
included in the NuGet package. This should be taken care of by
MsBuild automatically, but you may need to ensure your HLSL file
is a <SKShader /> item type in the .csproj for this to
work. You can also compile with the command line app manually if
you're compiling/distributing a shader some other way!
</div>

|  |  |
|--|--|
|string file|Path to a precompiled StereoKit Shader file!             If no .sks extension is part of this path, StereoKit will              automatically add it and check that first.|
|RETURNS: [Shader]({{site.url}}/Pages/Reference/Shader.html)|A shader from the given file, or null if it failed to load/compile.|




