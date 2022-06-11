---
layout: default
title: SpriteType
description: The way the Sprite is stored on the backend! Does it get batched and atlased for draw efficiency, or is it a single image?
---
# enum SpriteType

The way the Sprite is stored on the backend! Does it get
batched and atlased for draw efficiency, or is it a single image?

## Enum Values

|  |  |
|--|--|
|Atlased|The sprite will be batched onto an atlas texture so all sprites can be drawn in a single pass. This is excellent for performance! The only thing to watch out for here, adding a sprite to an atlas will rebuild the atlas texture! This can be a bit expensive, so it's recommended to add all sprites to an atlas at start, rather than during runtime. Also, if an image is too large, it may take up too much space on the atlas, and may be better as a Single sprite type.|
|Single|This sprite is on its own texture. This is best for large images, items that get loaded and unloaded during runtime, or for sprites that may have edge artifacts or severe 'bleed' from adjacent atlased images.|
