// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

using System;

namespace StereoKit
{
	/// <summary>Scene lighting! StereoKit's lighting is entirely environment
	/// based. An ambient lighting probe (`Ambient`) provides soft directional
	/// light, a specular reflection cubemap (`Reflection`) provides shiny
	/// highlights and mirror surfaces, and a dominant directional light
	/// (`MainLight`) is derived from it all for effects like shadows.
	/// `SetEnvironment` fills in all of this from a single cubemap, and is
	/// the easiest place to start!
	///
	/// On devices that can estimate lighting from their surroundings, the
	/// world source keeps all of this matched to the user's real room
	/// instead, see `Source`.</summary>
	public static class Lighting
	{
		/// <summary>Sets up the whole scene's lighting from a single cubemap!
		/// This includes the `Renderer.SkyboxTex`, a generated
		/// `Lighting.Reflection`, and once that's finished generating,
		/// `Ambient` and `MainLight` too. Raw radiance cubemaps like
		/// `Tex.FromCubemap` provides are perfect here, no mip chain needed.
		///
		/// Anything you assign to those properties afterwards overrides that
		/// piece. Null resets all of it to StereoKit's defaults. Ignored when
		/// using `World` as a lighting source.</summary>
		/// <param name="skyCubemap">A cubemap of the environment's radiance,
		/// such as one from `Tex.FromCubemap`, or null to reset to the
		/// default environment.</param>
		public static void SetEnvironment(Tex skyCubemap)
		{
			NativeAPI.lighting_set_environment(skyCubemap == null ? IntPtr.Zero : skyCubemap._inst, out IntPtr reflection);
			if (reflection != IntPtr.Zero) NativeAPI.tex_release(reflection);
		}

		/// <summary>This overload also hands back the reflection texture it
		/// generated, for hooking `Tex.OnLoaded`, or re-convolving later
		/// with `Tex.GenCubemapReflection`.</summary>
		/// <param name="skyCubemap">A cubemap of the environment's radiance,
		/// such as one from `Tex.FromCubemap`, or null to reset to the
		/// default environment.</param>
		/// <param name="reflection">The generated reflection, or null if none
		/// was generated, such as on a null reset, or when using `World` as a
		/// lighting source.</param>
		public static void SetEnvironment(Tex skyCubemap, out Tex reflection)
		{
			NativeAPI.lighting_set_environment(skyCubemap == null ? IntPtr.Zero : skyCubemap._inst, out IntPtr inst);
			reflection = inst == IntPtr.Zero ? null : new Tex(inst);
		}

		/// <summary>Where scene lighting comes from right now? The
		/// application (`Manual`, the default), or estimated live from the
		/// user's surroundings (`World`). This never changes on its own, the
		/// world source is an opt-in via `RequestSource`.
		///
		/// A request that's still settling reads as its previous value here,
		/// see `SourcePending`.</summary>
		public static LightingSource Source
			=> NativeAPI.lighting_get_source();

		/// <summary>True while a `RequestSource` is still settling, which is
		/// typically a permission dialog. Resolves within moments, with the
		/// outcome in `Source`.</summary>
		public static bool SourcePending
			=> NativeAPI.lighting_source_pending();

		/// <summary>Requests a switch to a lighting source! `Manual` applies
		/// immediately, while `World` may show a permission dialog the user
		/// can decline, watch `SourcePending` and `Source` for the result.</summary>
		/// <param name="source">The lighting source to switch to.</param>
		public static void RequestSource(LightingSource source)
			=> NativeAPI.lighting_request_source(source);

		/// <summary>How environment lighting is delivered! `Ambient` folds
		/// all light into the `Ambient` SH, while `MainLight` splits the
		/// dominant directional light out into `MainLight`, leaving `Ambient`
		/// the remainder.
		///
		/// NOTE: StereoKit builtin shaders do not yet account for
		/// directional light, you will need your own shaders for `MainLight`
		/// mode to work. Changing `Mode` will ALSO change your _current_
		/// `Ambient` SH, adding or subtracting the MainLight's energy.</summary>
		public static LightingMode Mode
		{
			get => NativeAPI.lighting_get_mode();
			set => NativeAPI.lighting_set_mode(value);
		}

		/// <summary>The scene's dominant directional light, ideal as a
		/// shadow direction! Environment lighting keeps this derived, and
		/// the direction is always normalized. Check `Mode` before shading
		/// with it, since in `Ambient` mode this light's energy is _also_
		/// inside the `Ambient` probe.
		///
		/// A black color means no light. Assignments follow the same rules
		/// as `Ambient`: applied as-is until the next environment lighting
		/// replaces them. Built-in shaders don't consume this, it's for
		/// your own shaders and effects.</summary>
		public static SHLight MainLight
		{
			get => NativeAPI.lighting_get_main_light();
			set => NativeAPI.lighting_set_main_light(value);
		}
		/// <summary>The scene's ambient light probe, as spherical harmonics!
		/// This is soft omnidirectional light, the color and intensity
		/// arriving from each direction rather than a discrete source.
		/// Build one with `SphericalHarmonics.FromLights`, or let
		/// `SetEnvironment` derive it from a cubemap.
		///
		/// Assignments apply exactly as provided, and win over a
		/// `SetEnvironment` that's still loading, even once it finishes.
		/// Later environment lighting replaces them: a new `SetEnvironment`,
		/// or world source estimates, where assignments are ignored entirely.
		/// A `Mode` change re-shapes the newest lighting, assigned or not.</summary>
		public static SphericalHarmonics Ambient
		{
			get => NativeAPI.lighting_get_ambient();
			set => NativeAPI.lighting_set_ambient(value);
		}
		/// <summary>The specular reflection cubemap used by PBR shading: GGX
		/// convolved radiance, one roughness level per mip. Generate one
		/// with `Tex.GenCubemapReflection`, and null restores the built-in
		/// default.
		///
		/// A cubemap without a convolved mip chain still binds, but reads
		/// mirror-sharp at every roughness. In the world source this is fed
		/// by estimation, and assignments are ignored.</summary>
		public static Tex Reflection
		{
			get { IntPtr inst = NativeAPI.lighting_get_reflection(); return inst == IntPtr.Zero ? null : new Tex(inst); }
			set { NativeAPI.lighting_set_reflection(value != null ? value._inst : IntPtr.Zero); }
		}
		/// <summary>Check if a lighting source is supported on this device,
		/// without switching to it! Manual is always available, while world
		/// needs light estimation support from the XR runtime, and may still
		/// need a permission the user can decline at `RequestSource`
		/// time.</summary>
		/// <param name="source">The lighting source to check on.</param>
		/// <returns>True if the source can be used on this device.</returns>
		public static bool SourceAvailable(LightingSource source)
			=> NativeAPI.lighting_source_available(source);
	}
}
