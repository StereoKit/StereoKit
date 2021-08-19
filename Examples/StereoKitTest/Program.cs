using System;
using StereoKit;

class Program 
{
	static void Main(string[] args) 
	{
		// If the app has a constructor that takes a string array, then
		// we'll use that, and pass the command line arguments into it on
		// creation
		Type appType = typeof(App);
		App  app     = appType.GetConstructor(new Type[] { typeof(string[]) }) != null
			? (App)Activator.CreateInstance(appType, new object[] { args })
			: (App)Activator.CreateInstance(appType);
		if (app == null)
			throw new Exception("StereoKit loader couldn't construct an instance of the App!");

		// Initialize StereoKit, and the app
		if (!SK.Initialize(app.Settings))
			Environment.Exit(1);
		app.Init();

		// Now loop until finished, and then shut down
		Action step = app.Step; // Storing this separately reduces allocations
		while (SK.Step(step)) { }
		SK.Shutdown();
	}
}
