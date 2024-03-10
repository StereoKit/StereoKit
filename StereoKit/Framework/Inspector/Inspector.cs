using System;
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
		public Type MemberType => MemberInfoType(info);

		public static Type MemberInfoType(MemberInfo mi) => mi is FieldInfo field ? field.FieldType : (mi is PropertyInfo prop ? prop.PropertyType : null);
	}

	public partial class Inspector : IStepper
	{
		public static float LabelWidth = 0.06f;
		struct Item
		{
			public InspectorMember           info;
			public InspectorTypeAttribute    typeAttribute;
			public InspectorExtraAttribute[] extraAttributes;
			public bool                      readOnly;
		}
		struct TypeAttribute
		{
			public Type       type;
			public MethodInfo match;
		}

		Type            _type;
		object          _inst;
		Item[]          _items;
		Pose            _pose;
		TypeAttribute[] _defaultTypeAttrs;

		public Inspector()
		{
			// Find all InspectorAttribute Types in the assembly that have a Match method
			string       matchFn    = "Match";
			BindingFlags matchFlags = BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic;
			_defaultTypeAttrs = AppDomain.CurrentDomain.GetAssemblies()
				.SelectMany(a => a.GetTypes())
				.Where     (t => typeof(InspectorTypeAttribute).IsAssignableFrom(t) && t.GetMethod(matchFn, matchFlags) != null)
				.Select    (t => new TypeAttribute { type = t, match = t.GetMethod(matchFn, matchFlags) })
				.ToArray   ();
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
			if      (_type         == null) UI.Label("No type selected!");
			else if (_items.Length == 0   ) UI.Label("No inspectable fields!");
			else
			{
				for (int i=0; i<_items.Length; i+=1)
				{
					UI.PushId(i);
					UI.PushEnabled(_items[i].readOnly == false);
					foreach (var extra in _items[i].extraAttributes)
						extra.Draw(_inst);
					_items[i].typeAttribute?.Draw(_items[i].info, _inst);
					UI.PopEnabled();
					UI.PopId();
				}
			}
			UI.WindowEnd();
			UI.PopId();
		}

		static bool ValidMember(MemberInfo m, bool showPrivate)
			=>  (m is FieldInfo || (m is PropertyInfo pi && pi.GetGetMethod(true) != null)) &&
				(m.GetCustomAttribute<HideAttribute>() == null) &&
				(showPrivate || m.GetCustomAttribute<ShowAttribute>() != null || (m is FieldInfo fi && fi.IsPublic) || (m is PropertyInfo p && p.GetGetMethod(false) != null) || m.GetCustomAttributes().Any(a => a is InspectorTypeAttribute || a is InspectorExtraAttribute));

		static InspectorTypeAttribute GetMatchingTypeAttr(TypeAttribute[] from, Type type)
		{
			// Find the best matching attribute
			object[] args    = new object[] { type };
			int      max     = 0;
			Type     maxType = null;
			foreach (TypeAttribute t in from)
			{
				int currMax = (int)t.match.Invoke(null, args);
				if (max < currMax)
				{
					max = currMax;
					maxType = t.type;
				}
			}
			return maxType != null
				? (InspectorTypeAttribute)Activator.CreateInstance(maxType) 
				: null;
		}

		void SetObject(Type type, object instance, bool showPrivate)
		{
			BindingFlags flags = (instance == null ? BindingFlags.Static : BindingFlags.Instance) | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.FlattenHierarchy;

			_type  = type;
			_inst  = instance;
			_items = type
				.GetMembers(flags)
				.Where (m => ValidMember(m, showPrivate))
				.Select(m => new Item {
					info          = new InspectorMember(m),
					readOnly      = m is PropertyInfo pi && pi.CanWrite == false,
					typeAttribute = m
						.GetCustomAttributes()
						.Where (a => a is InspectorTypeAttribute)
						.Select(a => (InspectorTypeAttribute)a)
						.FirstOrDefault()
						?? GetMatchingTypeAttr(_defaultTypeAttrs, InspectorMember.MemberInfoType(m)),
					extraAttributes = m
						.GetCustomAttributes()
						.Where  (a => a is InspectorExtraAttribute)
						.Select (a => (InspectorExtraAttribute)a)
						.ToArray(),
					})
				.Where  (i => i.typeAttribute != null)
				.ToArray();
		}

		public static void Show(object target,       bool showPrivate = false) => SK.GetOrCreateStepper<Inspector>().SetObject(target?.GetType(), target, showPrivate);
		public static void Show(Type   staticTarget, bool showPrivate = false) => SK.GetOrCreateStepper<Inspector>().SetObject(staticTarget,      null,   showPrivate);

	}
}
