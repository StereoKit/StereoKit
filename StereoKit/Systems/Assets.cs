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
	}
}
