using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit.Framework
{
    public interface IStepper
    {
        bool Enabled { get; }

        bool Initialize();

        void Step();

        void Shutdown();
    }
}
