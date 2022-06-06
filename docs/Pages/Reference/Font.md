---
layout: default
title: Font
description: This class represents a text font asset! On the back-end, this asset is composed of a texture with font characters rendered to it, and a list of data about where, and how large those characters are on the texture.  This asset is used anywhere that text shows up, like in the UI or Text classes!
---
# class Font

This class represents a text font asset! On the back-end, this asset
is composed of a texture with font characters rendered to it, and a list of
data about where, and how large those characters are on the texture.

This asset is used anywhere that text shows up, like in the UI or Text classes!

## Static Fields and Properties

|  |  |
|--|--|
|[Font]({{site.url}}/Pages/Reference/Font.html) [Default]({{site.url}}/Pages/Reference/Font/Default.html)|The default font used by StereoKit's text. This varies from platform to platform, but is typically a sans-serif general purpose font, such as Segoe UI.|

## Static Methods

|  |  |
|--|--|
|[Find]({{site.url}}/Pages/Reference/Font/Find.html)|Searches the asset list for a font with the given Id, returning null if none is found.|
|[FromFile]({{site.url}}/Pages/Reference/Font/FromFile.html)|Loads a font and creates a font asset from it.|
