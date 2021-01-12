namespace StereoKit
{
	public interface ISKApp
	{
		SKSettings Settings { get; }
		void Init();
		void Step();
	}
}
