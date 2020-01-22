---
layout: default
title: FilePicker
description: Lets you pick files from the file system! Use FilePicker.Show to begin a file picker, and FilePicker.Hide to close it. You can safely call FilePicker.Hide even if it's already closed.
---
# FilePicker

Lets you pick files from the file system! Use FilePicker.Show
to begin a file picker, and FilePicker.Hide to close it. You can safely
call FilePicker.Hide even if it's already closed.




## Static Fields and Properties

|  |  |
|--|--|
|bool [Active]({{site.url}}/Pages/Reference/FilePicker/Active.html)|Is the file picker visible and doing things?|


## Static Methods

|  |  |
|--|--|
|[Hide]({{site.url}}/Pages/Reference/FilePicker/Hide.html)|If the file picker is `Active`, this will cancel it out and remove it from the stepper list. If it's already inactive, this does nothing!|
|[Show]({{site.url}}/Pages/Reference/FilePicker/Show.html)|Show a file picker to the user! If one is already up, it'll be cancelled out, and this one will replace it.|

