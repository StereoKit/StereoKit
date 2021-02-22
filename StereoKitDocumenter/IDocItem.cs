namespace StereoKitDocumenter
{
	interface IDocItem
	{
		string Name     { get; }
		string FileName { get; }
		string UrlName  { get; }

		void AddExample(DocExample aExample);
	}
}
