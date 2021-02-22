---
layout: default
title: Hand.Get
description: Returns the joint information of the indicated hand joint! This also includes fingertips as a 'joint'. This is the same as the [] operator. Note that for thumbs, there are only 4 'joints' in reality, so StereoKit has JointId.Root and JointId.KnuckleMajor as the same pose, so JointId.Tip is still the tip of the thumb!
---
# [Hand]({{site.url}}/Pages/Reference/Hand.html).Get

<div class='signature' markdown='1'>
[HandJoint]({{site.url}}/Pages/Reference/HandJoint.html) Get([FingerId]({{site.url}}/Pages/Reference/FingerId.html) finger, [JointId]({{site.url}}/Pages/Reference/JointId.html) joint)
</div>

|  |  |
|--|--|
|[FingerId]({{site.url}}/Pages/Reference/FingerId.html) finger|Which finger are we getting from here, 0 is             thumb, and pinky is 4!|
|[JointId]({{site.url}}/Pages/Reference/JointId.html) joint|Which joint on the finger are we getting? 0              is the root, which is all the way at the base of the palm, and 4             is the tip, the very end of the finger.|
|RETURNS: [HandJoint]({{site.url}}/Pages/Reference/HandJoint.html)|Position, orientation, and radius of the finger joint.|

Returns the joint information of the indicated hand
joint! This also includes fingertips as a 'joint'. This is the
same as the [] operator. Note that for thumbs, there are only 4
'joints' in reality, so StereoKit has JointId.Root and
JointId.KnuckleMajor as the same pose, so JointId.Tip is still
the tip of the thumb!
<div class='signature' markdown='1'>
[HandJoint]({{site.url}}/Pages/Reference/HandJoint.html) Get(int finger, int joint)
</div>

|  |  |
|--|--|
|int finger|Which finger are we getting from here, 0 is             thumb, and pinky is 4!|
|int joint|Which joint on the finger are we getting? 0              is the root, which is all the way at the base of the palm, and 4             is the tip, the very end of the finger.|
|RETURNS: [HandJoint]({{site.url}}/Pages/Reference/HandJoint.html)|Position, orientation, and radius of the finger joint.|

Returns the joint information of the indicated hand
joint! This also includes fingertips as a 'joint'. This is the
same as the [] operator. Note that for thumbs, there are only 4
'joints' in reality, so StereoKit has JointId.Root and
JointId.KnuckleMajor as the same pose, so JointId.Tip is still
the tip of the thumb!



