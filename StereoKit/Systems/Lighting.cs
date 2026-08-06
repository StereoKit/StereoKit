// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

using System;

namespace StereoKit;

/// <summary>Scene lighting! StereoKit's lighting is entirely environment
/// based: an ambient lighting probe (`Ambient`) provides soft directional
/// light, and a specular reflection cubemap (`Reflection`) provides
/// shiny highlights and mirror surfaces. `SetEnvironment` fills in all
/// of this from a single cubemap, and is the easiest place to start!
///
/// On devices that can estimate lighting from their surroundings, world
/// mode keeps all of this matched to the user's real room instead, see
/// `Mode`.</summary>
public static class Lighting
{
    /// <summary>Set up the whole scene environment from a single cubemap!
    /// The skybox backdrop will show it, a specular `Reflection` is
    /// generated from it, and once that reflection is ready, its lighting
    /// data is applied to `Ambient` as well. Raw radiance cubemaps, like
    /// those `Tex.FromCubemap` provides, are perfect here, and need no
    /// mip chain.
    ///
    /// This is a convenience over assigning `Renderer.SkyTex`,
    /// `Lighting.Reflection`, and `Lighting.Ambient` individually, and
    /// anything you assign to those afterwards overrides that piece. In
    /// world lighting mode this call is ignored, since lighting comes
    /// from the device's sensors there.</summary>
    /// <param name="skyCubemap">A cubemap texture representing the
    /// environment's radiance, such as one from `Tex.FromCubemap`.</param>
    public static void SetEnvironment(Tex skyCubemap)
    {
        NativeAPI.lighting_set_environment(skyCubemap._inst, out IntPtr reflection);
        if (reflection != IntPtr.Zero) NativeAPI.tex_release(reflection);
    }

    /// <summary>This overload also hands back the reflection texture it
    /// generated, for hooking `Tex.OnLoaded` or re-convolving later with
    /// `Tex.GenCubemapReflection`. See the other overload for the full
    /// story.</summary>
    /// <param name="skyCubemap">A cubemap texture representing the
    /// environment's radiance, such as one from `Tex.FromCubemap`.</param>
    /// <param name="reflection">The reflection generated from the
    /// cubemap, or null if the call did nothing, such as in world
    /// lighting mode.</param>
    public static void SetEnvironment(Tex skyCubemap, out Tex reflection)
    {
        NativeAPI.lighting_set_environment(skyCubemap._inst, out IntPtr inst);
        reflection = inst == IntPtr.Zero ? null : new Tex(inst);
    }

    /// <summary>Where does scene lighting come from? In manual mode, the
    /// application provides it via this class. In world mode, it's
    /// estimated live from the user's surroundings via the device's
    /// sensors, and assignments to `Ambient` and `Reflection` are
    /// ignored. The default is auto, which picks world mode when the
    /// display is transparent and the device supports estimation, and
    /// manual mode otherwise.
    ///
    /// World mode may require a permission the user can decline, so
    /// setting it is a request, and the value read back reflects what
    /// was actually possible. `ModeAvailable` checks support without
    /// switching.</summary>
    public static LightingMode Mode
    {
        get => NativeAPI.lighting_get_mode();
        set => NativeAPI.lighting_set_mode(value);
    }
    /// <summary>The ambient light probe for the scene, as spherical
    /// harmonics! This is soft omnidirectional lighting: think of it as
    /// the color and intensity of the light arriving from each
    /// direction, rather than a discrete light source. You can build one
    /// from a list of directional lights with
    /// `SphericalHarmonics.FromLights`, or from an environment via
    /// `SetEnvironment` or a reflection's `Tex.CubemapLighting`.
    ///
    /// Assigning this overrides any ambient that a `SetEnvironment` call
    /// would derive, so set the environment first, then your override.
    /// In world lighting mode this is managed by the system, and
    /// assignments are ignored.</summary>
    public static SphericalHarmonics Ambient
    {
        get => NativeAPI.lighting_get_ambient();
        set => NativeAPI.lighting_set_ambient(value);
    }
    /// <summary>The specular reflection cubemap used by PBR shading: GGX
    /// convolved radiance with one roughness level per mip. Generate one
    /// from an environment cubemap with `Tex.GenCubemapReflection`. A
    /// cubemap without a convolved mip chain will still bind, but reads as
    /// mirror-sharp at every roughness. In world lighting mode this is
    /// managed by the system, and assignments are ignored.
    ///
    /// Assigning a reflection sets only the reflection. If you want
    /// ambient lighting derived from it too, `SetEnvironment` does
    /// that.</summary>
    public static Tex Reflection
    {
        get { IntPtr inst = NativeAPI.lighting_get_reflection(); return inst == IntPtr.Zero ? null : new Tex(inst); }
        set { NativeAPI.lighting_set_reflection(value != null ? value._inst : IntPtr.Zero); }
    }
    /// <summary>Check if a lighting mode is supported on this device
    /// without switching to it! Manual and auto are always available,
    /// while world mode needs light estimation support from the XR
    /// runtime. Note that world mode may also need a permission the user
    /// can still decline when you set `Mode`.</summary>
    /// <param name="mode">The lighting mode to check on.</param>
    /// <returns>True if the mode can be used on this device.</returns>
    public static bool ModeAvailable(LightingMode mode)
        => NativeAPI.lighting_mode_available(mode);
}