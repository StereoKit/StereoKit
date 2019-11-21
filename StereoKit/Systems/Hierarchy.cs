using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
    public static class Hierarchy
    {
        public static void Push(in Matrix parentTransform)
            => NativeAPI.hierarchy_push(parentTransform);
        public static void Pop()
            => NativeAPI.hierarchy_pop();

        public static bool Enabled {
            get => NativeAPI.hierarchy_is_enabled();
            set => NativeAPI.hierarchy_set_enabled(value);
        }
    }
}
