---
layout: default
title: Log
description: A class for logging errors, warnings and information! Different levels of information can be filtered out, and supports coloration via <~[colorCode]> and <~clr> tags.  Text colors can be set with a tag, and reset back to default with <~clr>. Color codes are as follows.  | Dark | Bright | Decription  | |------|--------|-------------| | DARK | BRIGHT | DESCRIPTION | | blk  | BLK    | Black       | | red  | RED    | Red         | | grn  | GRN    | Green       | | ylw  | YLW    | Yellow      | | blu  | BLU    | Blue        | | mag  | MAG    | Magenta     | | cyn  | cyn    | Cyan        | | grn  | GRN    | Green       | | wht  | WHT    | White       |
---
# Log

A class for logging errors, warnings and information! Different levels
of information can be filtered out, and supports coloration via <~[colorCode]> and
<~clr> tags.

Text colors can be set with a tag, and reset back to default with <~clr>. Color
codes are as follows:

| Dark | Bright | Decription  |
|------|--------|-------------|
| DARK | BRIGHT | DESCRIPTION |
| blk  | BLK    | Black       |
| red  | RED    | Red         |
| grn  | GRN    | Green       |
| ylw  | YLW    | Yellow      |
| blu  | BLU    | Blue        |
| mag  | MAG    | Magenta     |
| cyn  | cyn    | Cyan        |
| grn  | GRN    | Green       |
| wht  | WHT    | White       |


## Fields and Properties

|  |  |
|--|--|
|[LogLevel]({{site.url}}/Pages/Reference/LogLevel.html) [Filter]({{site.url}}/Pages/Reference/Log/Filter.html)|What's the lowest level of severity logs to display on the console? Default is LogLevel.Info.|



## Methods

|  |  |
|--|--|
|[Err]({{site.url}}/Pages/Reference/Log/Err.html)|Writes a formatted line to the log using a LogLevel.Error severity level!|
|[Info]({{site.url}}/Pages/Reference/Log/Info.html)|Writes a formatted line to the log using a LogLevel.Info severity level!|
|[Warn]({{site.url}}/Pages/Reference/Log/Warn.html)|Writes a formatted line to the log using a LogLevel.Warn severity level!|
|[Write]({{site.url}}/Pages/Reference/Log/Write.html)|Writes a formatted line to the log with the specified severity level!|


