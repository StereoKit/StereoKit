---
layout: default
title: Sound.CreateStream
description: Create a sound used for streaming audio in or out! This is useful for things like reading from a microphone stream, or playing audio from a source streaming over the network, or even procedural sounds that are generated on the fly!  Use stream sounds with the WriteSamples and ReadSamples functions.
---
# [Sound]({{site.url}}/Pages/Reference/Sound.html).CreateStream

<div class='signature' markdown='1'>
```csharp
static Sound CreateStream(float streamBufferDuration)
```
Create a sound used for streaming audio in or out! This
is useful for things like reading from a microphone stream, or
playing audio from a source streaming over the network, or even
procedural sounds that are generated on the fly!

Use stream sounds with the WriteSamples and ReadSamples
functions.
</div>

|  |  |
|--|--|
|float streamBufferDuration|How much audio time should             this stream be able to hold without writing back over itself?|
|RETURNS: [Sound]({{site.url}}/Pages/Reference/Sound.html)|A stream sound that can be read and written to.|




