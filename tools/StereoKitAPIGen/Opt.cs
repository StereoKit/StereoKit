namespace StereoKitAPIGen;

public class Opt<T> where T : struct
{
	public T Val { get; set; }

	public Opt(T value)
	{
		Val = value;
	}
}