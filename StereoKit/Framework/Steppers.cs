using System;
using System.Collections.Generic;

namespace StereoKit.Framework
{
	internal class Steppers
	{
		List<IStepper> _steppers = new List<IStepper>();

		public void Shutdown()
		{
			_steppers.ForEach(s => s.Shutdown());
			_steppers.Clear();
		}

		public T Add<T>() where T : IStepper
		{ 
			T inst = Activator.CreateInstance<T>(); 
			Add(inst); 
			return inst;
		}
		public object Add(Type type)
		{
			IStepper inst = Activator.CreateInstance(type) as IStepper;
			if (inst == null) return null;
			Add(inst);
			return inst;
		}
		public T Add<T>(T stepper) where T:IStepper 
		{
			// Add the stepper to the list
			_steppers.Add(stepper);

			// And initialize the stepper!
			if (SK.IsInitialized)
				stepper.Initialize();

			return stepper;
		}
		public void InitializeSteppers()
		{
			// This should only be called for steppers that were added before
			// initialization!
			System.Diagnostics.Debug.Assert(!SK.IsInitialized);
			_steppers.ForEach(s => s.Initialize());
		}

		public void Remove<T>() => Remove(typeof(T));
		public void Remove(Type type)
		{
			_steppers.RemoveAll(s => {
				bool remove = type.IsAssignableFrom(s.GetType());
				if (remove) s.Shutdown();
				return remove;
			});
		}
		public void Remove(IStepper stepper)
		{
			if (_steppers.Remove(stepper))
				stepper.Shutdown();
		}


		public void Step()
		{
			for (int i = 0; i < _steppers.Count; i++)
			{
				_steppers[i].Step();
			}
		}
	}
}
