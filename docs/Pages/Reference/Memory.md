---
layout: default
title: Memory
description: For performance sensitive areas, or places dealing with large chunks of memory, it can be faster to get a reference to that memory rather than copying it! However, if this isn't explicitly stated, it isn't necessarily clear what's happening. So this enum allows us to visibly specify what type of memory reference is occurring.
---
# enum Memory

For performance sensitive areas, or places dealing with large chunks of
memory, it can be faster to get a reference to that memory rather than
copying it! However, if this isn't explicitly stated, it isn't necessarily
clear what's happening. So this enum allows us to visibly specify what type
of memory reference is occurring.

## Enum Values

|  |  |
|--|--|
|Copy|This memory is now _yours_ and you must free it yourself! Memory has been allocated, and the data has been copied over to it. Pricey! But safe.|
|Reference|The chunk of memory involved here is a reference that is still managed or used by StereoKit! You should _not_ free it, and be extremely cautious about modifying it.|
