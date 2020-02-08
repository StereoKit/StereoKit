---
layout: default
title: Sound.FromFile
description: Loads a sound effect from file! Currently, StereoKit only supports .wav files. Audio is converted to mono.
---
# [Sound]({{site.url}}/Pages/Reference/Sound.html).FromFile

<div class='signature' markdown='1'>
static [Sound]({{site.url}}/Pages/Reference/Sound.html) FromFile(string filename)
</div>

|  |  |
|--|--|
|string filename|Name of the audio file! Supports .wav files.|
|RETURNS: [Sound]({{site.url}}/Pages/Reference/Sound.html)|A sound object, or null if something went wrong.|

Loads a sound effect from file! Currently, StereoKit only supports
.wav files. Audio is converted to mono.




## Examples

### Basic usage
```csharp
Sound sound = Sound.FromFile("BlipNoise.wav");
sound.Play(Vec3.Zero);
```

