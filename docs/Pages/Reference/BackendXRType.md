---
layout: default
title: BackendXRType
description: This describes what technology is being used to power StereoKit's XR backend.
---
# enum BackendXRType

This describes what technology is being used to power StereoKit's
XR backend.

## Enum Values

|  |  |
|--|--|
|None|StereoKit is not using an XR backend of any sort. That means the application is flatscreen and has the simulator disabled.|
|OpenXR|StereoKit is currently powered by OpenXR! This means we're running on a real XR device. Not all OpenXR runtimes provide the same functionality, but we will have access to more fun stuff :)|
|Simulator|StereoKit is using the flatscreen XR simulator. Inputs are emulated, and some advanced XR functionality may not be available.|
|WebXR|StereoKit is running in a browser, and is using WebXR!|
