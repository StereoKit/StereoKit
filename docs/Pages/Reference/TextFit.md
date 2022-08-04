---
layout: default
title: TextFit
description: This enum describes how text layout behaves within the space it is given.
---
# enum TextFit

This enum describes how text layout behaves within the space
it is given.

## Enum Values

|  |  |
|--|--|
|Clip|When the text reaches the end, it is simply truncated and no longer visible.|
|Exact|If the text is larger, or smaller than the space provided, it will scale down or up to fill the space.|
|Overflow|The text will ignore the containing space, and just keep on going.|
|Squeeze|If the text is too large to fit in the space provided, it will be scaled down to fit inside. This will not scale up.|
|Wrap|The text will wrap around to the next line down when it reaches the end of the space on the X axis.|
