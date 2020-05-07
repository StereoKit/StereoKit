namespace StereoKit.Framework
{
	/// <summary>This class represents a single layer in the HandRadialMenu.
	/// Each item in the layer is displayed around the radial menu's circle.</summary>
	public class HandRadialLayer
	{
		/// <summary>Name of the layer, this is used for layer traversal, so
		/// make sure you get the spelling right! Perhaps use const strings
		/// for these.</summary>
		public string         layerName;
		/// <summary>A list of menu items to display in this menu layer.</summary>
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

		/// <summary>Creates a menu layer, this overload will calculate a backAngle
		/// if there are any back actions present in the item list.</summary>
		/// <param name="name">Name of the layer, this is used for layer traversal, so
		/// make sure you get the spelling right! Perhaps use const strings
		/// for these.</param>
		/// <param name="items">A list of menu items to display in this menu layer.</param>
		public HandRadialLayer(string name, params HandMenuItem[] items)
			:this(name, 0, items)
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
		/// <summary>Creates a menu layer with an angle offset for the layer's rotation!</summary>
		/// <param name="name">Name of the layer, this is used for layer traversal, so
		/// make sure you get the spelling right! Perhaps use const strings
		/// for these.</param>
		/// <param name="startAngle">An angle offset for the layer, if you want a specific
		/// orientation for the menu's contents. Note this may not behave as
		/// expected if you're setting this manually and using the backAngle 
		/// as well.</param>
		/// <param name="items">A list of menu items to display in this menu layer.</param>
		public HandRadialLayer(string name, float startAngle, params HandMenuItem[] items)
		{
			this.layerName  = name;
			this.startAngle = startAngle;
			this.items      = items;
		}
	}
}
