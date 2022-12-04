using System;
using System.Collections.Concurrent;
using System.Collections.Generic;

namespace StereoKit.Framework
{
	internal class Steppers
	{
		List           <IStepper> _steppers = new List<IStepper>();
		ConcurrentQueue<IStepper> _remove   = new ConcurrentQueue<IStepper>();
		ConcurrentQueue<IStepper> _add      = new ConcurrentQueue<IStepper>();

		public void Shutdown()
		{
			_steppers.ForEach(s => s.Shutdown());
			_steppers.Clear();
		}

		public T Add<T>() where T : IStepper
		{ 
			T inst = Activator.CreateInstance<T>();
			_add.Enqueue(inst);
			return inst;
		}
		public object Add(Type type)
		{
			IStepper inst = Activator.CreateInstance(type) as IStepper;
			if (inst == null) return null;
			_add.Enqueue(inst);
			return inst;
		}
		public T Add<T>(T stepper) where T:IStepper 
		{
			_add.Enqueue(stepper);
			return stepper;
		}

		public void Remove<T>() => Remove(typeof(T));
		public void Remove(Type type)
		{
			foreach(IStepper s in _steppers)
			{
				if (type.IsAssignableFrom(s.GetType()))
					_remove.Enqueue(s);
			}
		}
		public void Remove(IStepper stepper)
		{
			_remove.Enqueue(stepper);
		}


		public void Step()
		{
			while (_add.TryDequeue(out IStepper a))
			{
				a.Initialize();
				_steppers.Add(a);
			}
			while (_remove.TryDequeue(out IStepper r))
			{
				_steppers.Remove(r);
				r.Shutdown();
			}

			for (int i = 0; i < _steppers.Count; i++)
			{
				_steppers[i].Step();
			}
		}
	}
}
