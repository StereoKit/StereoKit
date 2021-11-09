---
layout: default
title: Sound.FromFile
description: Loads a sound effect from file! Currently, StereoKit supports .wav and .mp3 files. Audio is converted to mono.
---
# [Sound]({{site.url}}/Pages/Reference/Sound.html).FromFile

<div class='signature' markdown='1'>
```csharp
static Sound FromFile(string filename)
```
Loads a sound effect from file! Currently, StereoKit
supports .wav and .mp3 files. Audio is converted to mono.
</div>

|  |  |
|--|--|
|string filename|Name of the audio file! Supports .wav and             .mp3 files.|
|RETURNS: [Sound]({{site.url}}/Pages/Reference/Sound.html)|A sound object, or null if something went wrong.|





## Examples

### Basic usage
```csharp
Sound sound = Sound.FromFile("BlipNoise.wav");
sound.Play(Vec3.Zero);
```

