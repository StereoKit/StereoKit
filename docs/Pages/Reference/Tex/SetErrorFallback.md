---
layout: default
title: Tex.SetErrorFallback
description: This is the texture that all Tex objects with errors will fall back to. Assigning a texture here that isn't fully loaded will cause the app to block until it is loaded.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).SetErrorFallback

<div class='signature' markdown='1'>
```csharp
static void SetErrorFallback(Tex errorTexture)
```
This is the texture that all Tex objects with errors will
fall back to. Assigning a texture here that isn't fully loaded will
cause the app to block until it is loaded.
</div>

|  |  |
|--|--|
|[Tex]({{site.url}}/Pages/Reference/Tex.html) errorTexture|Any _valid_ texture here is fine.             Preferably loaded already, but doesn't have to be.|




