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

		List           <IStepper>      _steppers = new List<IStepper>();
		ConcurrentQueue<StepperAction> _actions  = new ConcurrentQueue<StepperAction>();

		public void Shutdown()
		{
			_steppers.ForEach(s => s.Shutdown());
			_steppers.Clear();
		}

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
			_actions.Enqueue(new StepperAction(stepper, ActionType.Add));
			return stepper;
		}

		public void Remove<T>() => Remove(typeof(T));
		public void Remove(Type type)
		{
			foreach(IStepper s in _steppers)
			{
				if (type.IsAssignableFrom(s.GetType()))
					_actions.Enqueue(new StepperAction(s, ActionType.Remove));
			}
		}
		public void Remove(IStepper stepper)
		{
			_actions.Enqueue(new StepperAction(stepper, ActionType.Remove));
		}


		public void Step()
		{
			while (_actions.TryDequeue(out StepperAction action))
			{
				switch (action.type)
				{
					case ActionType.Add:
						action.stepper.Initialize();
						_steppers.Add(action.stepper);
						break;
					case ActionType.Remove:
						_steppers.Remove(action.stepper);
						action.stepper.Shutdown();
						break;
				}
			}

			foreach (IStepper stepper in _steppers)
				stepper.Step();
		}
	}
}
