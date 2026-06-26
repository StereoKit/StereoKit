using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Reflection;

namespace StereoKit.Framework
{
	internal class Steppers
	{
		enum   ActionType { Add, Remove }
		struct StepperAction
		{
			public IStepper   stepper;
			public ActionType type;
			public StepperAction(IStepper stepper, ActionType type) { this.stepper = stepper; this.type = type; }
		}
		struct StepperItem
		{
			public IStepper stepper;
			public int      priority;
			public StepperItem(IStepper stepper, int priority) { this.stepper = stepper; this.priority = priority; }
		}
		readonly object                _stepperLock = new object();
		List           <StepperItem>   _steppers    = new List<StepperItem>();
		ConcurrentQueue<StepperAction> _actions     = new ConcurrentQueue<StepperAction>();
		// _steppers is sorted ascending by priority, and this is the index of
		// the first stepper with a priority greater than zero. It's the boundary
		// between the pre-app steppers [0.._postApp) and the post-app steppers
		// [_postApp..Count), maintained as items are inserted and removed.
		int                            _postApp     = 0;

		public IEnumerable<IStepper> steppers
		{
			get
			{
				foreach (StepperItem item in _steppers)
					yield return item.stepper;
			}
		}

		// Add steppers via the threadsafe action queue
		public T Add<T>() where T : IStepper
		{
			T inst = Activator.CreateInstance<T>();
			_actions.Enqueue(new StepperAction(inst, ActionType.Add));
			return inst;
		}
		public object Add(Type type)
		{
			IStepper inst = Activator.CreateInstance(type) as IStepper;
			if (inst == null) return null;
			_actions.Enqueue(new StepperAction(inst, ActionType.Add));
			return inst;
		}
		public T Add<T>(T stepper) where T:IStepper
		{
			if (stepper == null) throw new ArgumentNullException();
			_actions.Enqueue(new StepperAction(stepper, ActionType.Add));
			return stepper;
		}

		// Remove steppers via the threadsafe action queue
		public void Remove<T>() => Remove(typeof(T));
		public void Remove(Type type)
		{
			lock (_stepperLock)
			{
				foreach (StepperItem item in _steppers)
				{
					if (type.IsAssignableFrom(item.stepper.GetType()))
						_actions.Enqueue(new StepperAction(item.stepper, ActionType.Remove));
				}
			}
		}
		public void Remove(IStepper stepper)
		{
			if (stepper != null)
				_actions.Enqueue(new StepperAction(stepper, ActionType.Remove));
		}

		public T Get<T>() => (T)Get(typeof(T));
		public object Get(Type type)
		{
			lock (_stepperLock)
			{
				foreach (StepperItem item in _steppers)
				{
					if (type.IsAssignableFrom(item.stepper.GetType()))
						return item.stepper;
				}
			}
			foreach (StepperAction action in _actions)
			{
				if (type.IsAssignableFrom(action.stepper.GetType()))
					return action.stepper;
			}
			return null;
		}

		public void DequeueActions()
		{
			// Execute all stepper actions on the main thread in the order they
			// were given.
			while (_actions.TryDequeue(out StepperAction action))
			{
				switch (action.type)
				{
					case ActionType.Add:
						if (action.stepper.Initialize())
						{
							// Read the priority from the optional [StepperPriority]
							// attribute, defaulting to 0 when it's absent.
							StepperPriorityAttribute attr = action.stepper.GetType().GetCustomAttribute<StepperPriorityAttribute>(true);
							StepperItem item = new StepperItem(action.stepper, attr?.Priority ?? 0);

							// Insert keeping _steppers sorted ascending by priority.
							// This is stable: an item is placed after any existing
							// items with an equal priority, preserving add order.
							lock (_stepperLock)
							{
								int i = _steppers.Count;
								while (i > 0 && _steppers[i - 1].priority > item.priority)
									i -= 1;
								_steppers.Insert(i, item);
								// A pre-app item pushes the boundary forward.
								if (item.priority <= 0) _postApp += 1;
							}
						}
						break;
					case ActionType.Remove:
						bool result = false;
						lock (_stepperLock)
						{
							// Pull the boundary back for each pre-app item removed.
							result = _steppers.RemoveAll(i =>
							{
								if (i.stepper != action.stepper) return false;
								if (i.priority <= 0) _postApp -= 1;
								return true;
							}) > 0;
						}
						if (result) action.stepper.Shutdown();
						break;
				}
			}
		}

		// Steps all ISteppers with a priority less than or equal to zero. This
		// runs before the app's main Step callback. New stepper Add/Remove
		// actions are also processed here, at the start of the frame.
		public void StepPreApp()
		{
			DequeueActions();

			// _steppers only changes via DequeueActions on this thread, so the
			// list and _postApp boundary are stable for the rest of this frame.
			for (int i = 0; i < _postApp; i += 1)
				_steppers[i].stepper.Step();
		}

		// Steps all ISteppers with a priority greater than zero. This runs after
		// the app's main Step callback.
		public void StepPostApp()
		{
			int count = _steppers.Count;
			for (int i = _postApp; i < count; i += 1)
				_steppers[i].stepper.Step();
		}

		public void Shutdown()
		{
			_actions = new ConcurrentQueue<StepperAction>();
			_steppers.ForEach(i => i.stepper.Shutdown());
			_steppers.Clear();
			_postApp = 0;
		}
	}
}
