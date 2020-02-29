#if WINDOWS_UWP
using Microsoft.MixedReality.QR;
using StereoKit;
using System;
using System.Collections.Generic;

class DemoQRCode : ITest
{
    QRCodeWatcher watcher;
    Dictionary<Guid, Pose> poses = new Dictionary<Guid, Pose>();

    public void Initialize()
    {
        QRCodeWatcher.RequestAccessAsync().Wait();
        watcher = new QRCodeWatcher();
        watcher.Added   += (o, q) => poses.Add(q.Code.Id, QRToPose(q.Code));
        watcher.Updated += (o, q) => poses[q.Code.Id] = QRToPose(q.Code);
        watcher.Removed += (o, q) => poses.Remove(q.Code.Id);
        watcher.Start();
    }

    public void Shutdown()
    {
        watcher?.Stop();
    }

    public void Update()
    {
        foreach(Pose p in poses.Values)
            Lines.AddAxis(p);
    }

    static Pose QRToPose(QRCode qr)
    {
        Pose result = Pose.FromSpatialNode(qr.SpatialGraphNodeId.ToByteArray());
        return result;
    }
}
#endif