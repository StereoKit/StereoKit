---
layout: default
title: Log.Filter
description: What's the lowest level of severity logs to display on the console? Default is LogLevel.Info. This property can safely be set before SK initialization.
---
# [Log]({{site.url}}/Pages/Reference/Log.html).Filter

<div class='signature' markdown='1'>
static [LogLevel]({{site.url}}/Pages/Reference/LogLevel.html) Filter{ set }
</div>

## Description
What's the lowest level of severity logs to display on
the console? Default is LogLevel.Info. This property can safely
be set before SK initialization.


## Examples

Show everything that StereoKit logs!
```csharp
Log.Filter = LogLevel.Diagnostic;
```
Or, only show warnings and errors:
```csharp
Log.Filter = LogLevel.Warning;
```

