using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace StereoKit.Framework
{
    internal class Steppers
    {
        struct StepperInfo : IComparable<StepperInfo>
        {
            public IStepper stepper;
            public int sortId;

            public int CompareTo(StepperInfo other)
                => sortId.CompareTo(other.sortId);
        }
        List<StepperInfo>     _steppers = new List<StepperInfo>();
        Dictionary<Type, int> _stepperSort;

        static IEnumerable<Type> GetLoadableTypes(Assembly a)
        {
            if (a == null) throw new ArgumentNullException("assembly");
            try { 
                return a.GetTypes();
            } catch (ReflectionTypeLoadException e) { 
                return e.Types.Where(t => t != null);
            }
        }
        static bool IsAssignable(Type interfaceType, Type otherType)
        {
            try {
                return interfaceType.IsAssignableFrom(otherType);
            } catch(Exception) {
                // It's expected for IsAssignableFrom to raise exceptions in UWP? .Net Native?
                // during normal operation. Expected a TypeLoadException, but was also getting
                // an <unknown> exception.
                return false;
            }
        }

        public bool Initialize()
        {
            Type stepperType = typeof(IStepper);
            Type[] stepperTypes = AppDomain.CurrentDomain.GetAssemblies()
                .SelectMany(assembly => GetLoadableTypes(assembly)
                    .Where(type => IsAssignable(stepperType, type)))
                .ToArray();

            // TODO: change this to a dependency graph sort order!
            _stepperSort = new Dictionary<Type, int>();
            for (int i = 0; i < stepperTypes.Length; i++)
                _stepperSort[stepperTypes[i]] = i;

            return true;
        }
        public void Shutdown()
        {
            _steppers.ForEach(s => s.stepper.Shutdown());
            _steppers.Clear();
        }

        public T Add<T>() where T : IStepper
        { 
            T inst = Activator.CreateInstance<T>(); 
            Add(inst); 
            return inst;
        }
        public T Add<T>(T stepper) where T:IStepper 
        {
            // Check the sort order for this stepper type
            int sortId = -1;
            if (!_stepperSort.TryGetValue(stepper.GetType(), out sortId))
            {
                Log.Err($"Couldn't find a sort order for stepper: {stepper.GetType().Name}");
                return default(T);
            }

            // Add the stepper to the list
            StepperInfo info = new StepperInfo { 
                stepper = stepper, 
                sortId  = sortId };
            int index = _steppers.BinarySearch(info);
            if (index < 0) index = ~index;
            _steppers.Insert(index, info);

            // And initialize the stepper!
            stepper.Initialize();

            return stepper;
        }
        
        public void Remove<T>() where T:IStepper
            => _steppers.RemoveAll(s=> typeof(T).IsAssignableFrom(s.stepper.GetType()));
        public void Remove(IStepper stepper)
            => _steppers.RemoveAt(_steppers.FindIndex(s=>s.stepper == stepper));
        

        public void Step()
        {
            for (int i = 0; i < _steppers.Count; i++)
            {
                _steppers[i].stepper.Step();
            }
        }
    }
}
