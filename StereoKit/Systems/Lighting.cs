// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

using System;

namespace StereoKit;

public static class Lighting
{
    public static LightingMode Mode
    {
        get => NativeAPI.lighting_get_mode();
        set => NativeAPI.lighting_set_mode(value);
    }
    public static SphericalHarmonics Ambient
    {
        get => NativeAPI.lighting_get_ambient();
        set => NativeAPI.lighting_set_ambient(value);
    }
    public static Tex Reflection
    {
        get { IntPtr inst = NativeAPI.lighting_get_reflection(); return inst == IntPtr.Zero ? null : new Tex(inst); }
        set { NativeAPI.lighting_set_reflection(value != null ? value._inst : IntPtr.Zero); }
    }
    public static void SetDirectional(Vec3 direction, Color colorLinear)
        => NativeAPI.lighting_set_directional(direction, colorLinear);
    public static void GetDirectional(out Vec3 direction, out Color colorLinear)
        => NativeAPI.lighting_get_directional(out direction, out colorLinear);
    public static bool ModeAvailable(LightingMode mode)
        => NativeAPI.lighting_mode_available(mode);
}