using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace StereoKit.Framework
{
	public interface InspectorTypeAttribute
	{
		void Draw(InspectorMember info, object inst);
	}
	public interface InspectorExtraAttribute
	{
		void Draw(object inst);
	}

	public struct InspectorMember
	{
		public MemberInfo info;
		public InspectorMember(MemberInfo info)
		{
			this.info = info;
		}
		public T GetValue<T>(object inst)
		{
			if      (info is FieldInfo    field) return (T)field.GetValue(inst);
			else if (info is PropertyInfo prop ) return (T)prop .GetValue(inst);
			else                                 return default;
		}
		public void SetValue<T>(object inst, T value)
		{
			if      (info is FieldInfo    field) field.SetValue(inst, value);
			else if (info is PropertyInfo prop ) prop .SetValue(inst, value);
		}
		public Type MemberType => info is FieldInfo field ? field.FieldType : (info is PropertyInfo prop ? prop.PropertyType : null);
	}

	public partial class Inspector : IStepper
	{
		public static float LabelWidth = 0.06f;
		struct Item
		{
			public InspectorMember           info;
			public InspectorTypeAttribute    typeAttribute;
			public InspectorExtraAttribute[] extraAttributes;
		}

		Type       _type;
		object     _inst;
		List<Item> _items;
		Pose       _pose;
		List<Type> _defaultInspectorAttributes = new List<Type>();

		public Inspector()
		{
			// Find all InspectorAttribute Types in the assembly that have a Match method
			_defaultInspectorAttributes = AppDomain.CurrentDomain.GetAssemblies()
				.SelectMany(a => a.GetTypes())
				.Where     (t => typeof(InspectorTypeAttribute).IsAssignableFrom(t) && t.GetMethod("Match", BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic) != null)
				.ToList();
		}

		bool IStepper.Initialize()
		{
			_pose = UI.PopupPose();
			return true;
		}

		void IStepper.Shutdown()
		{
		}

		void IStepper.Step()
		{
			UI.PushId("SK Inspector");
			UI.WindowBegin(_type == null ? "Inspector" : _type.Name, ref _pose);
			if (_items == null || _items.Count == 0) UI.Label("No object selected!");
			else
			{
				int i = 1;
				foreach (Item item in _items)
				{
					UI.PushId(i++);
					UI.PushEnabled(item.info.info is PropertyInfo pi && pi.CanWrite || item.info.info is FieldInfo);
					foreach (var extra in item.extraAttributes)
						extra.Draw(_inst);
					item.typeAttribute?.Draw(item.info, _inst);
					UI.PopEnabled();
					UI.PopId();
				}
			}
			UI.WindowEnd();
			UI.PopId();
		}

		void SetObject(Type type, object instance, bool showPrivate)
		{
			BindingFlags flags = (instance == null ? BindingFlags.Static : BindingFlags.Instance) | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.FlattenHierarchy;

			_type  = type;
			_inst  = instance;
			_items = type.GetMembers(flags)
				.Where (m => m is FieldInfo || (m is PropertyInfo pi && pi.GetGetMethod() != null))
				.Where (m =>
					showPrivate || m.GetCustomAttribute<ShowAttribute>() != null ||
					(m is FieldInfo fi && fi.IsPublic) ||
					m.GetCustomAttributes().Any(a => a is InspectorTypeAttribute || a is InspectorExtraAttribute))
				.Where(m => m.GetCustomAttribute<HideAttribute>() == null)
				.Select(m => new Item {
					info       = new InspectorMember(m),
					typeAttribute = m.GetCustomAttributes()
						.Where (a => a is InspectorTypeAttribute)
						.Select(a => (InspectorTypeAttribute)a)
						.FirstOrDefault(),
					extraAttributes = m.GetCustomAttributes()
						.Where  (a => a is InspectorExtraAttribute)
						.Select (a => (InspectorExtraAttribute)a)
						.ToArray()
					})
				.ToList();

			// If there are no attributes, try to see if one of the defaults
			// match it.
			for (int i = 0; i < _items.Count; i++)
			{
				if (_items[i].typeAttribute != null) continue;

				// Find the best matching default attribute
				object[] args    = new object[] { _items[i].info.MemberType };
				int      max     = 0;
				Type     maxType = null;
				foreach (Type t in _defaultInspectorAttributes)
				{
					int currMax = (int)t.GetMethod("Match", BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic).Invoke(null, args);
					if (max < currMax)
					{
						max = currMax;
						maxType = t;
					}
				}

				// If we found a match, create an instance of it, otherwise
				// remove the whole item from the list.
				if (maxType != null)
				{
					Item item = _items[i];
					item.typeAttribute = (InspectorTypeAttribute)Activator.CreateInstance(maxType);
					_items[i] = item;
				}
				else
				{
					_items.RemoveAt(i); i--;
				}
			}
		}

		public static void Show(object target,       bool showPrivate = false) => SK.GetOrCreateStepper<Inspector>().SetObject(target?.GetType(), target, showPrivate);
		public static void Show(Type   staticTarget, bool showPrivate = false) => SK.GetOrCreateStepper<Inspector>().SetObject(staticTarget,      null,   showPrivate);

	}
}
