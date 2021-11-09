---
layout: default
title: Log.Subscribe
description: Allows you to listen in on log events! Any callback subscribed here will be called when something is logged. This does honor the Log.Filter, so filtered logs will not be received here. This method can safely be called before SK initialization.
---
# [Log]({{site.url}}/Pages/Reference/Log.html).Subscribe

<div class='signature' markdown='1'>
```csharp
static void Subscribe(LogCallback onLog)
```
Allows you to listen in on log events! Any callback
subscribed here will be called when something is logged. This
does honor the Log.Filter, so filtered logs will not be received
here. This method can safely be called before SK initialization.
</div>

|  |  |
|--|--|
|LogCallback onLog|The function to call when a log event occurs.|





## Examples

Then you add the OnLog method into the log events like this in
your initialization code!
```csharp
Log.Subscribe(OnLog);
```
And in your Update loop, you can draw the window.
```csharp
LogWindow();
```
And that's it!
### An in-application log window
Here's an example of using the Log.Subscribe method to build a simple
logging window. This can be pretty handy to have around somewhere in
your application!

Here's the code for the window, and log tracking.
```csharp
static Pose         logPose = new Pose(0, -0.1f, 0.5f, Quat.LookDir(Vec3.Forward));
static List<string> logList = new List<string>();
static string       logText = "";
static void OnLog(LogLevel level, string text)
{
	if (logList.Count > 15)
		logList.RemoveAt(logList.Count - 1);
	logList.Insert(0, text.Length < 100 ? text : text.Substring(0,100)+"...\n");

	logText = "";
	for (int i = 0; i < logList.Count; i++)
		logText += logList[i];
}
static void LogWindow()
{
	UI.WindowBegin("Log", ref logPose, new Vec2(40, 0) * U.cm);
	UI.Text(logText);
	UI.WindowEnd();
}
```

