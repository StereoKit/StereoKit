using System;
using System.Collections.Generic;

namespace StereoKit.Framework
{
	/// <summary>This class represents a single layer in the HandRadialMenu.
	/// Each item in the layer is displayed around the radial menu's circle.
	/// </summary>
	public class HandRadialLayer
	{
		/// <summary>Name of the layer, this is used for layer traversal, so
		/// make sure you get the spelling right! Perhaps use const strings
		/// for these.</summary>
		public string         layerName;
		/// <summary>A list of menu items to display in this menu layer.
		/// </summary>
		public HandMenuItem[] items;
		/// <summary>An angle offset for the layer, if you want a specific
		/// orientation for the menu's contents. Note this may not behave as
		/// expected if you're setting this manually and using the backAngle 
		/// as well.</summary>
		public float          startAngle;
		/// <summary>What's the angle pointing towards the back button on this 
		/// menu? If there is a back button. This is used to orient the back
		/// button towards the direction the finger just came from.</summary>
		public float          backAngle = 0;

		internal HandRadialLayer       parent;
		internal List<HandRadialLayer> childLayers = new List<HandRadialLayer>();
		internal HandMenuItem          layerItem;

		public HandRadialLayer Parent => parent;

		/// <summary>Creates a menu layer, this overload will calculate a
		/// backAngle if there are any back actions present in the item list.
		/// </summary>
		/// <param name="name">Name of the layer, this is used for layer
		/// traversal, so make sure you get the spelling right! Perhaps use
		/// const strings for these.</param>
		/// <param name="items">A list of menu items to display in this menu
		/// layer.</param>
		public HandRadialLayer(string name, params HandMenuItem[] items)
			: this(name, null, items) { }

		/// <summary>Creates a menu layer, this overload will calculate a
		/// backAngle if there are any back actions present in the item list.
		/// </summary>
		/// <param name="name">Name of the layer, this is used for layer
		/// traversal, so make sure you get the spelling right! Perhaps use
		/// const strings for these.</param>
		/// <param name="items">A list of menu items to display in this menu
		/// layer.</param>
		public HandRadialLayer(string name, Sprite image, params HandMenuItem[] items)
			: this(name, image, 0, items)
		{
			for (int i = 0; i < items.Length; i++)
			{
				if (items[i].action == HandMenuAction.Back)
				{
					float step = (360.0f / items.Length);
					backAngle = (i+0.5f) * step;
				}
			}
		}

		/// <summary>Creates a menu layer with an angle offset for the layer's
		/// rotation!</summary>
		/// <param name="name">Name of the layer, this is used for layer
		/// traversal, so make sure you get the spelling right! Perhaps use
		/// const strings for these.</param>
		/// <param name="startAngle">An angle offset for the layer, if you want
		/// a specific orientation for the menu's contents. Note this may not
		/// behave as expected if you're setting this manually and using the
		/// backAngle as well.</param>
		/// <param name="items">A list of menu items to display in this menu
		/// layer.</param>
		public HandRadialLayer(string name, Sprite image, float startAngle, params HandMenuItem[] items)
		{
			this.layerName  = name;
			this.startAngle = startAngle;
			this.items      = items;
			this.layerItem  = new HandMenuItem(name, image, null, name);
		}

		/// <summary>This adds a menu layer as a child item of this layer.
		/// </summary>
		/// <param name="layer">Any HandRadialLayer or derivative.</param>
		public void AddChild(HandRadialLayer layer)
		{
			layer.parent = this;
			childLayers.Add(layer);
			AddItem(layer.layerItem);
		}

		/// <summary>Find a child menu layer by name.</summary>
		/// <param name="name">The exact name of the layer.</param>
		/// <returns>The layer with the matching name, or null.</returns>
		public HandRadialLayer FindChild(string name)
			=> childLayers.Find(l => l.layerName == name);

		/// <summary>Finds the layer in the list of child layers, and removes
		/// it, if it exists.</summary>
		/// <param name="layer">The exact layer in the layer list.</param>
		public void RemoveChild(HandRadialLayer layer)
		{
			childLayers.Remove(layer);
			RemoveItem(layer.layerItem);
		}

		/// <summary>This appends a new menu item to the end of the menu's
		/// list.</summary>
		/// <param name="item">Any HandMenuItem or derivative.</param>
		public void AddItem(HandMenuItem item)
		{
			Array.Resize(ref items, items.Length + 1);
			items[items.Length - 1] = item;
		}

		/// <summary>Find a menu item by name.</summary>
		/// <param name="name">The exact name of the item.</param>
		/// <returns>The item with the matching name, or null.</returns>
		public HandMenuItem FindItem(string name) 
			=> Array.Find(items, i=>i.name == name);

		/// <summary>Finds the item in the list, and removes it, if it exists.
		/// </summary>
		/// <param name="item">The exact item in the layer list.</param>
		public void RemoveItem(HandMenuItem item)
		{
			int index = Array.IndexOf(items, item);
			if (index < 0) return;
			for (int i = index; i < items.Length-1; i++)
			{
				items[i] = items[i+1];
			}
			Array.Resize(ref items, items.Length-1);
		}
	}
}
