using StereoKit;

class DemoDocDebug : IDemo
{
    public void Initialize()
    {
        /// :CodeSample: Log.Write
        Log.Write(LogLevel.Info, "<~grn>{0:0.0}s<~clr> have elapsed since StereoKit start.", Time.Total);
        /// :End:

        /// :CodeSample: Log.Info
        Log.Info("<~grn>{0:0.0}s<~clr> have elapsed since StereoKit start.", Time.Total);
        /// :End:
        
        /// :CodeSample: Log.Warn
        Log.Warn("Warning! <~ylw>{0:0.0}s<~clr> have elapsed since StereoKit start!", Time.Total);
        /// :End:

        /// :CodeSample: Log.Err
        if (Time.Elapsedf > 0.017f)
            Log.Err("Oh no! Frame time (<~red>{0}<~clr>) has exceeded 17ms! There's no way we'll hit even 60 frames per second!", Time.Elapsedf);
        /// :End:
    }

    public void Shutdown(){}

    public void Update()
    {
    }
}