---
layout: default
title: TextContext
description: Soft keyboard layouts are often specific to the type of text that they're editing! This enum is a collection of common text contexts that SK can pass along to the OS's soft keyboard for a more optimal layout.
---
# enum TextContext

Soft keyboard layouts are often specific to the type of text that they're
editing! This enum is a collection of common text contexts that SK can pass
along to the OS's soft keyboard for a more optimal layout.

## Enum Values

|  |  |
|--|--|
|Number|Numbers and numerical values.|
|Password|This is a password, and should not be visible when typed!|
|Text|General text editing, this is the most common type of text, and would result in a 'standard' keyboard layout.|
|Uri|This text specifically represents some kind of URL/URI address.|
