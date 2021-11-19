---
layout: default
title: Sound.WriteSamples
description: Only works if this Sound is a stream type! This writes a number of audio samples to the sample buffer, and samples should be between -1 and +1. Streams are stored as ring buffers of a fixed size, so writing beyond the capacity of the ring buffer will overwrite the oldest samples.  StereoKit uses 48,000 samples per second of audio.
---
# [Sound]({{site.url}}/Pages/Reference/Sound.html).WriteSamples

<div class='signature' markdown='1'>
```csharp
void WriteSamples(Single[]& samples)
```
Only works if this Sound is a stream type! This writes
a number of audio samples to the sample buffer, and samples
should be between -1 and +1. Streams are stored as ring buffers
of a fixed size, so writing beyond the capacity of the ring
buffer will overwrite the oldest samples.

StereoKit uses 48,000 samples per second of audio.
</div>

|  |  |
|--|--|
|Single[]& samples|An array of audio samples, where each             sample is between -1 and +1.|

<div class='signature' markdown='1'>
```csharp
void WriteSamples(Single[]& samples, int sampleCount)
```
Only works if this Sound is a stream type! This writes
a number of audio samples to the sample buffer, and samples
should be between -1 and +1. Streams are stored as ring buffers
of a fixed size, so writing beyond the capacity of the ring
buffer will overwrite the oldest samples.

StereoKit uses 48,000 samples per second of audio.
</div>

|  |  |
|--|--|
|Single[]& samples|An array of audio samples, where each              sample is between -1 and +1.|
|int sampleCount|You can use this to write only a subset             of the samples in the array, rather than the entire array!|




