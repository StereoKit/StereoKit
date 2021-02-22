---
layout: default
title: Sound.Play
description: Plays the sound at the 3D location specified, using the volume parameter as an additional volume control option! Sound volume falls off from 3D location, and can also indicate direction and location through spatial audio cues. So make sure the position is where you want people to think it's from! Currently, if this sound is playing somewhere else, it'll be cancelled, and moved to this location.
---
# [Sound]({{site.url}}/Pages/Reference/Sound.html).Play

<div class='signature' markdown='1'>
void Play([Vec3]({{site.url}}/Pages/Reference/Vec3.html) at, float volume)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) at|World space location for the audio to play at.|
|float volume|Volume modifier for the effect! 1 means full             volume, and 0 means completely silent.|

Plays the sound at the 3D location specified, using the
volume parameter as an additional volume control option! Sound
volume falls off from 3D location, and can also indicate
direction and location through spatial audio cues. So make sure
the position is where you want people to think it's from!
Currently, if this sound is playing somewhere else, it'll be
cancelled, and moved to this location.




## Examples

### Basic usage
```csharp
Sound sound = Sound.FromFile("BlipNoise.wav");
sound.Play(Vec3.Zero);
```

