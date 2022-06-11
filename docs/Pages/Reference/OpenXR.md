---
layout: default
title: OpenXR
description: This class is NOT of general interest, unless you are trying to add support for some unusual OpenXR extension! StereoKit should do all the OpenXR work that most people will need. If you find yourself here anyhow for something you feel StereoKit should support already, please add a feature request on GitHub!  This class contains handles and methods for working directly with OpenXR. This may allow you to activate or work with OpenXR extensions that StereoKit hasn't implemented or exposed yet. Check that Backend.XRType is OpenXR before using any of this.  These properties may best be used with some external OpenXR binding library, but you may get some limited mileage with the API as provided here.
---
# static class OpenXR

This class is NOT of general interest, unless you are
trying to add support for some unusual OpenXR extension! StereoKit
should do all the OpenXR work that most people will need. If you
find yourself here anyhow for something you feel StereoKit should
support already, please add a feature request on GitHub!

This class contains handles and methods for working directly with
OpenXR. This may allow you to activate or work with OpenXR
extensions that StereoKit hasn't implemented or exposed yet. Check
that Backend.XRType is OpenXR before using any of this.

These properties may best be used with some external OpenXR
binding library, but you may get some limited mileage with the API
as provided here.

## Static Fields and Properties

|  |  |
|--|--|
|Int64 [EyesSampleTime]({{site.url}}/Pages/Reference/OpenXR/EyesSampleTime.html)|Type: XrTime. This is the OpenXR time of the eye tracker sample associated with the current value of .|
|UInt64 [Instance]({{site.url}}/Pages/Reference/OpenXR/Instance.html)|Type: XrInstance. StereoKit's instance handle, valid after SK.Initialize.|
|UInt64 [Session]({{site.url}}/Pages/Reference/OpenXR/Session.html)|Type: XrSession. StereoKit's current session handle, this will be valid after SK.Initialize, but the session may not be started quite so early.|
|UInt64 [Space]({{site.url}}/Pages/Reference/OpenXR/Space.html)|Type: XrSpace. StereoKit's primary coordinate space, valid after SK.Initialize, this will most likely be created from `XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT` or `XR_REFERENCE_SPACE_TYPE_LOCAL`.|
|Int64 [Time]({{site.url}}/Pages/Reference/OpenXR/Time.html)|Type: XrTime. This is the OpenXR time for the current frame, and is available after SK.Initialize.|

## Static Methods

|  |  |
|--|--|
|[AddCompositionLayer]({{site.url}}/Pages/Reference/OpenXR/AddCompositionLayer.html)|This allows you to add XrCompositionLayers to the list that StereoKit submits to xrEndFrame. You must call this every frame you wish the layer to be included.|
|[ExtEnabled]({{site.url}}/Pages/Reference/OpenXR/ExtEnabled.html)|This tells if an OpenXR extension has been requested and successfully loaded by the runtime. This MUST only be called after SK.Initialize.|
|[GetFunction]({{site.url}}/Pages/Reference/OpenXR/GetFunction.html)|This is basically `xrGetInstanceProcAddr` from OpenXR, you can use this to get and call functions from an extension you've loaded. This uses `Marshal.GetDelegateForFunctionPointer` to turn the result into a delegate that you can call.|
|[GetFunctionPtr]({{site.url}}/Pages/Reference/OpenXR/GetFunctionPtr.html)|This is basically `xrGetInstanceProcAddr` from OpenXR, you can use this to get and call functions from an extension you've loaded. You can use `Marshal.GetDelegateForFunctionPointer` to turn the result into a delegate that you can call.|
|[RequestExt]({{site.url}}/Pages/Reference/OpenXR/RequestExt.html)|Requests that OpenXR load a particular extension. This MUST be called before SK.Initialize. Note that it's entirely possible that your extension will not load on certain runtimes, so be sure to check ExtEnabled to see if it's available to use.|
