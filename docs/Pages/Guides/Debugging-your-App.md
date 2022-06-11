---
layout: default
title: Debugging your App
description: ### Set up for debugg...
---

# Debugging your App
### Set up for debugging
Since StereoKit's core is composed of native code, there are a few extra steps you can take to get better stack traces and debug information! The first is to make sure Visual Studio is set up to debug with native code. This varies across .Net versions, but generally the option can be found at Project->Properties->Debug->(Native code debugging).

You may also wish to disable "Just My Code" if you're trying to actually inspect how StereoKit's code is behaving. This can be found under Tools->Options->Debugging->General->Enable Just My Code, and uncheck it to make sure it's disabled.

StereoKit is set up with Source Link as of v0.3.5, which allows you to inspect StereoKit's code directly from the relevant commit of the main repository on GitHub. Note that distributed binaries are in release format, and may not 'step through' as nicely as a normal debug binary would.

### Check the Logs!
StereoKit outputs a lot of useful information in the logs, and there's a chance your issue may be logged there! When submitting an issue on the GitHub repository, including a copy of your logs can really help maintainers to understand what is or isn't happening.

All platforms will output the log through the standard debug output window, but you can also tap into the debug logs via [`Log.Subscribe`]({{site.url}}/Pages/Reference/Log/Subscribe.html). Check the docs there for an easy Mixed Reality log window you can add to your project.

### Ask for Help
We love to hear what problems you're running into! StereoKit is completely open source and has no analytics or surveillance tools embedded in it at all. If you have an issue, we won't know about it unless _you_ tell us, or we spot it ourselves!

The best place to ask for help will always be the [GitHub Issues](https://github.com/StereoKit/StereoKit/issues), or [GitHub Discussions](https://github.com/StereoKit/StereoKit/discussions) pages. Be sure to provide logs, platform information, and as many other details as may be relevant!

## Common Issues
Here's a short list of some common issues we've seen people ask about!

### XR_ERROR_FORM_FACTOR_UNAVAILABLE in the logs
This is a common and expected message that basically means that OpenXR can't find any headset attached to the system. Your headset is most likely unplugged, but may also indicate some other issue with your OpenXR runtime setup.

By default, StereoKit will fall back to the flatscreen simulator when this error message is encountered. This behavior can be configured in your `SKSettings`.

### StereoKit isn't loading my asset!
This may manifest as Null Reference Exceptions surrounding your Model/Tex/asset. The first thing to do is check the StereoKit logs, and look for messages with your asset's filename. There will likely be some message with a decent hint available.

If StereoKit cannot find your file, make sure the path is correct, and verify your asset is correctly being copied into Visual Studio's output folder. The default templates will automatically copy content in the project's Assets folder into the final output folder. If your asset is not in the Assets folder, or if you have assembled your own project without using the templates, then you may need to do additional work to ensure the copy happens.

### System.DLLNotFoundException for StereoKitC
A StereoKit function has been called before the native StereoKit DLL was loaded. Make sure your code is happening _after_ your call to `SK.Initialize`! Watch out for code being called from implied constructors, especially on static classes.

For some rare cases where you need access to a StereoKit function before initialization, you may be able to call `SK.PreLoadLibrary`. This only works for functions that interact with code that does not require initialization, like math. It may also disguise code that's incorrectly being called before SK.Initialize.

