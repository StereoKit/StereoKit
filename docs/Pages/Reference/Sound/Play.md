---
layout: default
title: Sound.Play
description: Plays the sound at the 3D location specified, using the volume parameter as an additional volume control option! Sound volume falls off from 3D location, and can also indicate direction and location through spatial audio cues. So make sure the position is where you want people to think it's from! Currently, if this sound is playing somewhere else, it'll be canceled, and moved to this location.
---
# [Sound]({{site.url}}/Pages/Reference/Sound.html).Play

<div class='signature' markdown='1'>
```csharp
SoundInst Play(Vec3 at, float volume)
```
Plays the sound at the 3D location specified, using the
volume parameter as an additional volume control option! Sound
volume falls off from 3D location, and can also indicate
direction and location through spatial audio cues. So make sure
the position is where you want people to think it's from!
Currently, if this sound is playing somewhere else, it'll be
canceled, and moved to this location.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) at|World space location for the audio to play at.|
|float volume|Volume modifier for the effect! 1 means full             volume, and 0 means completely silent.|
|RETURNS: [SoundInst]({{site.url}}/Pages/Reference/SoundInst.html)|Returns a link to the Sound's play instance, which you can use to track and modify how the sound plays after the initial conditions are set.|





## Examples

### Basic usage
```csharp
Sound sound = Sound.FromFile("BlipNoise.wav");
sound.Play(Vec3.Zero);
```

