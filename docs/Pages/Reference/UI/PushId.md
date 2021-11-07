---
layout: default
title: UI.PushId
description: Adds a root id to the stack for the following UI elements! This id is combined when hashing any following ids, to prevent id collisions in separate groups.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).PushId

<div class='signature' markdown='1'>
static void PushId(string rootId)
</div>

|  |  |
|--|--|
|string rootId|The root id to use until the following PopId              call. MUST be unique within current hierarchy.|

Adds a root id to the stack for the following UI
elements! This id is combined when hashing any following ids, to
prevent id collisions in separate groups.
<div class='signature' markdown='1'>
static void PushId(int rootId)
</div>

|  |  |
|--|--|
|int rootId|The root id to use until the following PopId              call. MUST be unique within current hierarchy.|

Adds a root id to the stack for the following UI
elements! This id is combined when hashing any following ids, to
prevent id collisions in separate groups.



