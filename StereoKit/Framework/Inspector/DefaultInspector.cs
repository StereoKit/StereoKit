using System;

namespace StereoKit.Framework
{
	[Inspector.MatchType(types = null, priority = 1)]
	internal class DefaultInspector : IInspector
	{
		object        _inst;
		InspectItem[] _items;

		public void Initialize(Type type, object instance, InspectItem[] items)
		{
			_inst  = instance;
			_items = items;
		}

		public void Draw()
		{
			if (_items.Length == 0) { UI.Label("No inspectable fields!"); return; }

			for (int i=0; i<_items.Length; i+=1)
			{
				UI.PushId(i);
				_items[i].Draw(_inst);
				UI.PopId();
			}
		}
	}
}
