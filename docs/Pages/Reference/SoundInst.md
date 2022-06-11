---
layout: default
title: SoundInst
description: This represents a play instance of a Sound! You can get one when you call Sound.Play(). This allows you to do things like cancel a piece of audio early, or change the volume and position of it as it's playing.
---
# struct SoundInst

This represents a play instance of a Sound! You can get one
when you call Sound.Play(). This allows you to do things like cancel
a piece of audio early, or change the volume and position of it as
it's playing.

## Instance Fields and Properties

|  |  |
|--|--|
|bool [IsPlaying]({{site.url}}/Pages/Reference/SoundInst/IsPlaying.html)|Is this Sound instance currently playing? For streaming assets, this will be true even if they don't have any new data in them, and they're just idling at the end of their data.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [Position]({{site.url}}/Pages/Reference/SoundInst/Position.html)|The 3D position in world space this sound instance is currently playing at. If this instance is no longer valid, the position will be at zero.|
|float [Volume]({{site.url}}/Pages/Reference/SoundInst/Volume.html)|The volume multiplier of this Sound instance! A number between 0 and 1, where 0 is silent, and 1 is full volume.|

## Instance Methods

|  |  |
|--|--|
|[Stop]({{site.url}}/Pages/Reference/SoundInst/Stop.html)|This stops the sound early if it's still playing.|
