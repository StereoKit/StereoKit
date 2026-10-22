using System;
using System.Collections.Generic;
using System.Data;
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
	public interface IInspector
	{
		void Initialize(Type type, object instance, InspectItem[] items);
		void Draw();
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

	public struct InspectItem
	{
		public InspectorMember           info;
		public InspectorTypeAttribute    typeAttribute;
		public InspectorExtraAttribute[] extraAttributes;
		public bool                      readOnly;

		public void Draw(object inst)
		{
			UI.PushEnabled(readOnly == false);
			foreach (var extra in extraAttributes)
				extra.Draw(inst);
			typeAttribute?.Draw(info, inst);
			UI.PopEnabled();
		}
	}

	public partial class Inspector : IStepper
	{
		public static float LabelWidth => UI.LineHeight * 3;
		
		struct TypeAttribute
		{
			public Type               type;
			public MatchTypeAttribute matcher;
		}

		struct StackItem
		{
			public Type       type;
			public object     inst;
			public IInspector inspector;
		}

		Pose            _pose;
		TypeAttribute[] _defaultTypeAttrs;
		Type[]          _inspectorTypes;

		List<StackItem> _inspectionStack = new List<StackItem>();

		public Inspector()
		{
			IEnumerable<Type> assemblyTypes = AppDomain.CurrentDomain
				.GetAssemblies()
				.SelectMany(a => a.GetTypes());

			// Find all InspectorAttribute Types in the assembly that have a MatchType attribute
			_defaultTypeAttrs = assemblyTypes
				.Where  (t => typeof(InspectorTypeAttribute).IsAssignableFrom(t) && t.GetCustomAttribute<MatchTypeAttribute>() != null)
				.Select (t => new TypeAttribute { type = t, matcher = t.GetCustomAttribute<MatchTypeAttribute>() })
				.ToArray();

			_inspectorTypes = assemblyTypes
				.Where  (t => typeof(IInspector).IsAssignableFrom(t))
				.ToArray();
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
			if (_inspectionStack.Count > 0)
			{
				StackItem item = _inspectionStack[_inspectionStack.Count-1];
				UI.WindowBegin("Inspector", ref _pose);

				string parentName = _inspectionStack.Count > 1
					? _inspectionStack[_inspectionStack.Count-2].type.Name 
					: "Root";
				if (UI.ButtonImg(parentName, Sprite.ArrowLeft))
					_inspectionStack.RemoveAt(_inspectionStack.Count-1);
				UI.SameLine();
				UI.Label(item.type.Name);
				UI.HSeparator();

				item.inspector.Draw();
			} else {
				UI.WindowBegin("Inspector", ref _pose);
				UI.Label("No type selected!");
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
			int  max     = 0;
			Type maxType = null;
			foreach (TypeAttribute t in from)
			{
				int currMax = t.matcher.Match(type);
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

		static InspectItem[] GetInspectableItems(Type type, bool includePrivate, BindingFlags staticOrInstance, TypeAttribute[] additionalInspectors)
		{
			if (staticOrInstance != BindingFlags.Instance && staticOrInstance != BindingFlags.Static) throw new ArgumentException();
			BindingFlags flags = staticOrInstance | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.FlattenHierarchy;

			return type
				.GetMembers(flags)
				.Where (m => ValidMember(m, includePrivate))
				.Select(m => new InspectItem {
					info          = new InspectorMember(m),
					readOnly      = m is PropertyInfo pi && pi.CanWrite == false,
					typeAttribute = m
						.GetCustomAttributes()
						.Where (a => a is InspectorTypeAttribute)
						.Select(a => (InspectorTypeAttribute)a)
						.FirstOrDefault()
						?? GetMatchingTypeAttr(additionalInspectors, InspectorMember.MemberInfoType(m)),
					extraAttributes = m
						.GetCustomAttributes()
						.Where  (a => a is InspectorExtraAttribute)
						.Select (a => (InspectorExtraAttribute)a)
						.ToArray(),
					})
				.Where  (i => i.typeAttribute != null)
				.ToArray();
		}

		void SetObject(Type type, object instance, bool showPrivate)
		{
			_inspectionStack.Clear();
			PushObject(type, instance, showPrivate);
		}

		void PushObject(Type type, object instance, bool showPrivate)
		{
			StackItem item = new StackItem();
			item.type  = type;
			item.inst  = instance;

			Type inspectorMatch         = null;
			int  inspectorMatchPriority = int.MinValue;
			foreach (Type inspector in _inspectorTypes)
			{
				MatchTypeAttribute matcher = inspector.GetCustomAttribute<MatchTypeAttribute>();
				int                match   = matcher?.Match(type) ?? 0;
				if (inspectorMatchPriority < match)
				{
					inspectorMatchPriority = match;
					inspectorMatch         = inspector;
				}
			}
			item.inspector = Activator.CreateInstance(inspectorMatch) as IInspector;
			item.inspector.Initialize(type, instance, GetInspectableItems(type, showPrivate, instance == null ? BindingFlags.Static : BindingFlags.Instance, _defaultTypeAttrs));

			_inspectionStack.Add(item);
		}

		public static void Show(object target,       bool showPrivate = false) => SK.GetOrCreateStepper<Inspector>().SetObject (target?.GetType(), target, showPrivate);
		public static void Show(Type   staticTarget, bool showPrivate = false) => SK.GetOrCreateStepper<Inspector>().SetObject (staticTarget,      null,   showPrivate);
		public static void Push(object target,       bool showPrivate = false) => SK.GetOrCreateStepper<Inspector>().PushObject(target?.GetType(), target, showPrivate);
		public static void Push(Type   staticTarget, bool showPrivate = false) => SK.GetOrCreateStepper<Inspector>().PushObject(staticTarget,      null,   showPrivate);

	}
}
