using System;

namespace StereoKit.Framework
{
	/// <summary>This is a collection of display and behavior information for
	/// a single item on the hand menu.</summary>
	public class HandMenuItem
	{
		/// <summary>Display name of the item.</summary>
		public string name;
		/// <summary>Display image of the item, null is fine!</summary>
		public Sprite image;

		/// <summary>Describes the menu related behavior of this menu item,
		/// should it close the menu? Open another layer? Go back to the
		/// previous menu?</summary>
		public HandMenuAction action;
		/// <summary>The callback that should be performed when this menu
		/// item is selected.</summary>
		public Action callback;
		/// <summary>If the action is set to Layer, then this is the layer
		/// name used to find the next layer for the menu!</summary>
		public string layerName;

		/// <summary>A menu item that'll load another layer when selected.</summary>
		/// <param name="name">Display name of the item.</param>
		/// <param name="image">Display image of the item, null is fine!</param>
		/// <param name="callback">The callback that should be performed when this menu
		/// item is selected.</param>
		/// <param name="layerName">This is the layer name used to find the next layer 
		/// for the menu! Get the spelling right, try using const strings!</param>
		public HandMenuItem(string name, Sprite image, Action callback, string layerName) 
			: this(name, image, callback, HandMenuAction.Layer)
		{
			this.layerName = layerName;
		}
		/// <summary>Makes a menu item!</summary>
		/// <param name="name">Display name of the item.</param>
		/// <param name="image">Display image of the item, null is fine!</param>
		/// <param name="callback">The callback that should be performed when this menu
		/// item is selected.</param>
		/// <param name="action">Describes the menu related behavior of this menu item,
		/// should it close the menu? Open another layer? Go back to the
		/// previous menu?</param>
		public HandMenuItem(string name, Sprite image, Action callback, HandMenuAction action = HandMenuAction.Close)
		{
			this.name     = name;
			this.image    = image;
			this.action   = action;
			this.callback = callback;
		}
	}
}
