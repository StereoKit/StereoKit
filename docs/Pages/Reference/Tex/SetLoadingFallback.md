---
layout: default
title: Tex.SetLoadingFallback
description: This is the texture that all Tex objects will fall back to by default if they are still loading. Assigning a texture here that isn't fully loaded will cause the app to block until it is loaded.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).SetLoadingFallback

<div class='signature' markdown='1'>
```csharp
static void SetLoadingFallback(Tex loadingTexture)
```
This is the texture that all Tex objects will fall back to
by default if they are still loading. Assigning a texture here that
isn't fully loaded will cause the app to block until it is loaded.
</div>

|  |  |
|--|--|
|[Tex]({{site.url}}/Pages/Reference/Tex.html) loadingTexture|Any _valid_ texture here is fine.             Preferably loaded already, but doesn't have to be.|




