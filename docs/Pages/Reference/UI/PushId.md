---
layout: default
title: UI.PushId
description: Adds a root id to the stack for the following UI elements! This id is combined when hashing any following ids, to prevent id collisions in separate groups.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).PushId

<div class='signature' markdown='1'>
```csharp
static void PushId(string rootId)
```
Adds a root id to the stack for the following UI
elements! This id is combined when hashing any following ids, to
prevent id collisions in separate groups.
</div>

|  |  |
|--|--|
|string rootId|The root id to use until the following PopId              call. MUST be unique within current hierarchy.|

<div class='signature' markdown='1'>
```csharp
static void PushId(int rootId)
```
Adds a root id to the stack for the following UI
elements! This id is combined when hashing any following ids, to
prevent id collisions in separate groups.
</div>

|  |  |
|--|--|
|int rootId|The root id to use until the following PopId              call. MUST be unique within current hierarchy.|




