using System;
using System.Collections.Concurrent;
using System.Collections.Generic;

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
		readonly object                _stepperLock = new object();
		List           <IStepper>      _steppers    = new List<IStepper>();
		ConcurrentQueue<StepperAction> _actions     = new ConcurrentQueue<StepperAction>();

		public IEnumerable<IStepper> steppers { get { return _steppers; } }

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
				foreach (IStepper stepper in _steppers)
				{
					if (type.IsAssignableFrom(stepper.GetType()))
						_actions.Enqueue(new StepperAction(stepper, ActionType.Remove));
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
				foreach (IStepper stepper in _steppers)
				{
					if (type.IsAssignableFrom(stepper.GetType()))
						return stepper;
				}
			}
			foreach (StepperAction action in _actions)
			{
				if (type.IsAssignableFrom(action.stepper.GetType()))
					return action.stepper;
			}
			return null;
		}

		public void Step()
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
							lock (_stepperLock) _steppers.Add(action.stepper);
						}
						break;
					case ActionType.Remove:
						bool result = false;
						lock (_stepperLock) result = _steppers.Remove(action.stepper);
						if (result) action.stepper.Shutdown();
						break;
				}
			}

			foreach (IStepper stepper in _steppers)
				stepper.Step();
		}

		public void Shutdown()
		{
			_actions = new ConcurrentQueue<StepperAction>();
			_steppers.ForEach(s => s.Shutdown());
			_steppers.Clear();
		}
	}
}
