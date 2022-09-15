using System;
using System.Collections.Generic;

namespace StereoKit
{
	/// <summary>If you want to manage loading assets, this is the class for
	/// you!</summary>
	public static class Assets
	{
		internal struct CallbackData
		{
			public object              action;
			public AssetOnLoadCallback callback;
		}

		/// <summary>This is the index of the current asset loading task. Note
		/// that to load one asset, multiple tasks are generated.</summary>
		public static int CurrentTask => NativeAPI.assets_current_task();

		/// <summary>This is the total number of tasks that have been added to
		/// the loading system, including all completed and pending tasks. Note
		/// that to load one asset, multiple tasks are generated.</summary>
		public static int TotalTasks => NativeAPI.assets_total_tasks();

		/// <summary>StereoKit processes tasks in order of priority. This
		/// returns the priority of the current task, and can be used to wait
		/// until all tasks within a certain priority range have been
		/// completed.</summary>
		public static int CurrentTaskPriority => NativeAPI.assets_current_task_priority();

		/// <summary>This will block the execution of the application until
		/// all asset tasks below the priority value have completed loading.
		/// To block until all assets are loaded, pass in int.MaxValue for the
		/// priority.</summary>
		/// <param name="priority">Block the app until this priority level is
		/// complete.</param>
		public static void BlockForPriority(int priority) => NativeAPI.assets_block_for_priority(priority);

		/// <summary>A list of supported model format extensions. This pairs
		/// pretty well with `Platform.FilePicker` when attempting to load a
		/// `Model`!</summary>
		public static readonly string[] ModelFormats = { ".gltf", ".glb", ".obj", ".stl", ".ply" };
		/// <summary>A list of supported texture format extensions. This pairs
		/// pretty well with `Platform.FilePicker` when attempting to load a
		/// `Tex`!</summary>
		public static readonly string[] TextureFormats = { ".jpg", ".png", ".hdr", ".tga", ".bmp", ".psd", ".pic", ".qoi", ".gif" };

		private static AssetType TypeToAssetType(Type t)
		{
			switch (t)
			{
				case Type _ when t == typeof(Font    ): return AssetType.Font;
				case Type _ when t == typeof(Material): return AssetType.Material;
				case Type _ when t == typeof(Mesh    ): return AssetType.Mesh;
				case Type _ when t == typeof(Model   ): return AssetType.Model;
				case Type _ when t == typeof(Shader  ): return AssetType.Shader;
				case Type _ when t == typeof(Solid   ): return AssetType.Solid;
				case Type _ when t == typeof(Sound   ): return AssetType.Sound;
				case Type _ when t == typeof(Sprite  ): return AssetType.Sprite;
				case Type _ when t == typeof(Tex     ): return AssetType.Tex;
				case Type _ when t == typeof(IAsset  ): return AssetType.None;
				default: throw new ArgumentException("Not a valid asset type!");
			}
		}

		private static IAsset AssetTypeToAsset(AssetType currType, IntPtr inst)
		{
			switch ( currType )
			{
				case AssetType.Font:     return new Font    (inst);
				case AssetType.Material: return new Material(inst);
				case AssetType.Mesh:     return new Mesh    (inst);
				case AssetType.Model:    return new Model   (inst);
				case AssetType.Shader:   return new Shader  (inst);
				case AssetType.Solid:    return new Solid   (inst);
				case AssetType.Sound:    return new Sound   (inst);
				case AssetType.Sprite:   return new Sprite  (inst);
				case AssetType.Tex:      return new Tex     (inst);
				default: Log.Err("Found an invalid asset type!"); return null;
			}
		}

		public static IEnumerable<T> Type<T>() where T : IAsset
		{
			AssetType assetType = TypeToAssetType(typeof(T));
			int       count     = NativeAPI.assets_count();
			for (int i = 0; i < count; i++)
			{
				AssetType currType = NativeAPI.assets_get_type(i);
				if (assetType == AssetType.None || currType == assetType)
				{
					IntPtr inst  = NativeAPI.assets_get_index(i);
					IAsset asset = AssetTypeToAsset(currType, inst);
					if (asset != null)
						yield return (T)asset;
				}
			}
		}

		public static IEnumerable<IAsset> Type(Type type)
		{
			AssetType assetType = TypeToAssetType(type);
			int       count     = NativeAPI.assets_count();
			for (int i = 0; i < count; i++)
			{
				AssetType currType = NativeAPI.assets_get_type(i);
				if (assetType == AssetType.None || currType == assetType)
				{
					IntPtr inst  = NativeAPI.assets_get_index(i);
					IAsset asset = AssetTypeToAsset(currType, inst);
					if (asset != null)
						yield return asset;
				}
			}
		}
		
		public static IEnumerable<IAsset> All { get
		{
			int count = NativeAPI.assets_count();
			for (int i = 0; i < count; i++)
			{
				IntPtr inst  = NativeAPI.assets_get_index(i);
				IAsset asset = AssetTypeToAsset(NativeAPI.assets_get_type(i), inst);
				if (asset != null)
					yield return asset;
			}
		}}
	}
}
