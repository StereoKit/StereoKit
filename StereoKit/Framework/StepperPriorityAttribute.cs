using System;

namespace StereoKit.Framework
{
	/// <summary>An optional `[StepperPriority]` attribute for `IStepper` types
	/// that controls when and in what order their `Step` method is called
	/// relative to the app's main `Step` callback.
	///
	/// The priority value determines both the _phase_ and the _sort order_:
	/// `IStepper`s with a negative priority (or the default of 0) are stepped
	/// _before_ the app's main `Step` callback, and `IStepper`s with a positive
	/// priority are stepped _after_ it. In all cases, `IStepper`s are stepped in
	/// ascending order of priority, and ties preserve the order they were added
	/// in.
	///
	/// If an `IStepper` type does not have this attribute, it behaves as though
	/// it has a priority of 0.</summary>
	[AttributeUsage(AttributeTargets.Class, Inherited = true)]
	public sealed class StepperPriorityAttribute : Attribute
	{
		/// <summary>The priority value for this `IStepper`. Negative or zero
		/// values step before the app's main `Step` callback, positive values
		/// step after it, and all `IStepper`s are sorted in ascending order by
		/// this value.</summary>
		public int Priority { get; }

		/// <summary>Creates a priority attribute for an `IStepper` type.
		/// </summary>
		/// <param name="priority">The priority value. Negative or zero values
		/// step before the app's main `Step` callback, positive values step
		/// after it, and all `IStepper`s are sorted in ascending order by this
		/// value.</param>
		public StepperPriorityAttribute(int priority) { Priority = priority; }
	}
}
