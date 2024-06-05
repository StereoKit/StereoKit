// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

using System;

namespace StereoKit.Framework
{
	/// <summary> Don't know which to use? Try SoftOut! It's a great default!
	/// This site is a great reference for how these functions look:
	/// https://easings.net/
	/// </summary>
	public static class Ease
	{
		/// <summary>A constant motion the whole way through. Stops and starts
		/// hard, doesn't look all that great in most cases.</summary>
		public static float Linear        (float t) => t;

		/// <summary>Quadratic soft start with a hard stop.</summary>
		public static float SoftIn        (float t) => t * t;
		/// <summary>Quadratic fast start with a soft stop, a very good default
		/// for most cases as it feels very responsive and looks good.</summary>
		public static float SoftOut       (float t) => 1 - (1 - t) * (1 - t);
		/// <summary>Quadratic smooth start, fast middle, and smooth stop.
		/// </summary>
		public static float SoftInOut     (float t) => t < 0.5f ? 2 * t * t : 1 - (float)Math.Pow(-2 * t + 2, 2) / 2;


		/// <summary>Quartic soft start with a hard stop.</summary>
		public static float FastIn        (float t) => t * t * t * t;
		/// <summary>Quartic fast start with a soft stop, a pretty good default
		/// for most cases as it feels very responsive and looks good.</summary>
		public static float FastOut       (float t) => 1 - (float)Math.Pow(1 - t, 4);
		/// <summary>Quartic smooth start, fast middle, and smooth stop.
		/// </summary>
		public static float FastInOut     (float t) => t < 0.5 ? 8 * t * t * t * t : 1 - (float)Math.Pow(-2 * t + 2, 4) / 2;

		const float overshoot  = 1.70158f;
		const float overshoot2 = overshoot + 1;
		/// <summary>Soft start with a hard stop, overshoots its destination a
		/// bit before arriving at it.</summary>
		public static float OvershootIn   (float t) => overshoot2 * t * t * t - overshoot * t * t;
		/// <summary>Fast start with a soft stop, backs off a bit before moving
		/// to its destination.</summary>
		public static float OvershootOut  (float t) => 1 + overshoot2 * (float)Math.Pow(t - 1, 3) + overshoot * (float)Math.Pow(t - 1, 2);
		/// <summary>Smooth start, fast middle, and smooth stop. Overshoots on
		/// both the start and the end.</summary>
		public static float OvershootInOut(float t) => t < 0.5
		                                                 ? ((float)Math.Pow(2 * t,     2) * ((overshoot2 + 1) *  2 * t      - overshoot2)    ) / 2
		                                                 : ((float)Math.Pow(2 * t - 2, 2) * ((overshoot2 + 1) * (2 * t - 2) + overshoot2) + 2) / 2;
	}

	/// <summary>See the `Ease` class for a list of pre-made easing functions. An
	/// easing function that converts a value in the range of 0-1, to a different
	/// value mostly in the range of 0-1 with different properties.</summary>
	/// <param name="t">A value in the range of 0-1, usually linear.</param>
	/// <returns>A new value, mostly between 0-1.</returns>
	public delegate float EaseFn(float t);

	/// <summary>A value that can ease from one state to another over time on its
	/// own. Useful for quick, juicy animations and effects!</summary>
	public class EaseVec3
	{
		Vec3   begin, end;
		float  start, duration;
		EaseFn ease = Ease.Linear;

		/// <summary>Is this easing value currently easing from one state to
		/// the next, or is it finished and static?</summary>
		public bool IsFinished => Time.Totalf - start >= duration;

		/// <summary>Create a new ease value that starts with an initial
		/// value. This object will not animate until AnimTo is called.</summary>
		/// <param name="initial">The initial value for the object.</param>
		public EaseVec3(Vec3 initial) => SetTo(initial);
		/// <summary>Create a new ease value that starts with an initial
		/// value. This object will not animate until AnimTo is called.</summary>
		/// <param name="x">X component of a Vec3</param>
		/// <param name="y">Y component of a Vec3</param>
		/// <param name="z">Z component of a Vec3</param>
		public EaseVec3(float x, float y, float z) => SetTo(new Vec3(x,y,z));

		/// <summary>This will resolve the easing animation to its current
		/// value based on the current StereoKit time, and the state of the
		/// easing animation. This value is not cached internally.</summary>
		/// <returns>The current easing value.</returns>
		public Vec3 Resolve()
		{
			float t = Math.Min(1, (Time.Totalf - start) / duration);
			return Vec3.Lerp(begin, end, ease(t));
		}

		/// <summary>This sets up an easing animation for this value. It will
		/// now ease from its current state, to its destination state over a
		/// duration of time.</summary>
		/// <param name="destination">This is the final state for the value
		/// when the easing animation has finished.</param>
		/// <param name="duration">The duration of the easing animation in
		/// seconds.</param>
		/// <param name="easeFn">An easing function the animation will follow.
		/// A good all-around default would be Ease.SoftOut. A number of easing
		/// functions can be found in the Ease class.</param>
		public void AnimTo(Vec3 destination, float duration, EaseFn easeFn) { begin = Resolve(); start = Time.Totalf; end = destination; ease = easeFn; this.duration = duration; }
		/// <inheritdoc cref="AnimTo(Vec3, float, EaseFn)"/>
		/// <param name="x">X component of a Vec3</param>
		/// <param name="y">Y component of a Vec3</param>
		/// <param name="z">Z component of a Vec3</param>
		public void AnimTo(float x, float y, float z, float duration, EaseFn easeFn) => AnimTo(new Vec3(x, y, z), duration, easeFn);
		/// <summary>Sets this to a specific value with no easing animation.
		/// This will cancel any in-progress animations, and jump straight to
		/// the provided value.</summary>
		/// <param name="value">The new value that should be assigned right now
		/// without animation.</param>
		public void SetTo (Vec3 value) { begin = value; start = 0; end = value; ease = Ease.Linear; this.duration = 1; }

		/// <summary>An implicit "Resolve" of this easing type to its value.</summary>
		/// <param name="value">The easing value.</param>
		public static implicit operator Vec3(EaseVec3 value) => value.Resolve();
	}

	/// <inheritdoc cref="EaseVec3"/>
	public class EaseColor
	{
		Color  begin, end;
		float  start, duration;
		EaseFn ease = Ease.Linear;

		/// <inheritdoc cref="EaseVec3.IsFinished"/>
		public bool IsFinished => Time.Totalf - start >= duration;

		/// <inheritdoc cref="EaseVec3(Vec3)"/>
		public EaseColor(Color initial) => SetTo(initial);
		/// <inheritdoc cref="EaseVec3(Vec3)"/>
		/// <param name="r">Red component of a Color, in the range of 0-1.</param>
		/// <param name="g">Green component of a Color, in the range of 0-1.</param>
		/// <param name="b">Blue component of a Color, in the range of 0-1.</param>
		/// <param name="a">Alpha/transparency component of a Color, in the range of 0-1.</param>
		public EaseColor(float r, float g, float b, float a=1) => SetTo(new Color(r,g,b,a));

		/// <inheritdoc cref="EaseVec3.Resolve"/>
		public Color Resolve()
		{
			float t = Math.Min(1, (Time.Totalf - start) / duration);
			return Color.Lerp(begin, end, ease(t));
		}

		/// <inheritdoc cref="EaseVec3.AnimTo(Vec3, float, EaseFn)"/>
		public void AnimTo(Color destination, float duration, EaseFn easeFn) { begin = Resolve(); start = Time.Totalf; end = destination; ease = easeFn;      this.duration = duration; }
		/// <inheritdoc cref="EaseVec3.SetTo(Vec3)"/>
		public void SetTo (Color value)                                      { begin = value;     start = 0;           end = value;       ease = Ease.Linear; this.duration = 1; }

		/// <summary>An implicit "Resolve" of this easing type to its value.</summary>
		/// <param name="value">The easing value.</param>
		public static implicit operator Color(EaseColor value) => value.Resolve();
	}

	/// <inheritdoc cref="EaseVec3"/>
	public class EasePose
	{
		Pose   begin, end;
		float  start, duration;
		EaseFn ease = Ease.Linear;

		/// <inheritdoc cref="EaseVec3.IsFinished"/>
		public bool IsFinished => Time.Totalf - start >= duration;

		/// <inheritdoc cref="EaseVec3(Vec3)"/>
		public EasePose(Pose initial) => SetTo(initial);
		/// <inheritdoc cref="EaseVec3(Vec3)"/>
		/// <param name="pos">Position of a Pose.</param>
		/// <param name="rot">Rotation of a Pose.</param>
		public EasePose(Vec3 pos, Quat rot) => SetTo(new Pose(pos, rot));

		/// <inheritdoc cref="EaseVec3.Resolve"/>
		public Pose Resolve()
		{
			float t = Math.Min(1, (Time.Totalf - start) / duration);
			return Pose.Lerp(begin, end, ease(t));
		}

		/// <inheritdoc cref="EaseVec3.AnimTo(Vec3, float, EaseFn)"/>
		public void AnimTo(Pose destination, float duration, EaseFn easeFn) { begin = Resolve(); start = Time.Totalf; end = destination; ease = easeFn;      this.duration = duration; }
		/// <inheritdoc cref="EaseVec3.SetTo(Vec3)"/>
		public void SetTo (Pose value)                                      { begin = value;     start = 0;           end = value;       ease = Ease.Linear; this.duration = 1; }

		/// <summary>An implicit "Resolve" of this easing type to its value.</summary>
		/// <param name="value">The easing value.</param>
		public static implicit operator Pose(EasePose value) => value.Resolve();
	}
}