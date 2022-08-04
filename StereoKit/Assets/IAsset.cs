namespace StereoKit
{
	/// <summary>All StereoKit assets implement this interface! This is mostly
	/// to help group and hold Asset objects, and is particularly useful when
	/// working with Assets at a high level with the `Assets` class.</summary>
	public interface IAsset
	{
		/// <summary>Gets or sets the unique identifier of this asset resource!
		/// This can be helpful for debugging, managine your assets, or finding
		/// them later on!</summary>
		string Id { get; set; }
	}
}