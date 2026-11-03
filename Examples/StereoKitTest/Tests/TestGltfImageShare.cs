using StereoKit;
using System;
using System.Collections.Generic;
using System.Text;

class TestGltfImageShare : ITest
{
	// Images that point at the same buffer range should share one texture,
	// while an identical copy elsewhere in the buffer stays its own texture.
	bool SharedRangesShareTex()
	{
		byte[] png    = Platform.ReadFileBytes("toggle_on.png");
		byte[] buffer = new byte[36 + png.Length * 2];
		float[] verts = { 0,0,0,  1,0,0,  0,1,0 };
		Buffer.BlockCopy(verts, 0, buffer, 0, 36);
		Buffer.BlockCopy(png,   0, buffer, 36,              png.Length);
		Buffer.BlockCopy(png,   0, buffer, 36 + png.Length, png.Length);

		string json = $@"{{
			""asset"": {{ ""version"": ""2.0"" }},
			""scene"": 0, ""scenes"": [{{ ""nodes"": [0] }}], ""nodes"": [{{ ""mesh"": 0 }}],
			""buffers"": [{{ ""byteLength"": {buffer.Length}, ""uri"": ""data:application/octet-stream;base64,{Convert.ToBase64String(buffer)}"" }}],
			""bufferViews"": [
				{{ ""buffer"": 0, ""byteOffset"": 0,                   ""byteLength"": 36 }},
				{{ ""buffer"": 0, ""byteOffset"": 36,                  ""byteLength"": {png.Length} }},
				{{ ""buffer"": 0, ""byteOffset"": 36,                  ""byteLength"": {png.Length} }},
				{{ ""buffer"": 0, ""byteOffset"": {36 + png.Length}, ""byteLength"": {png.Length} }} ],
			""accessors"": [{{ ""bufferView"": 0, ""componentType"": 5126, ""count"": 3, ""type"": ""VEC3"", ""min"": [0,0,0], ""max"": [1,1,0] }}],
			""images"": [
				{{ ""bufferView"": 1, ""mimeType"": ""image/png"" }},
				{{ ""bufferView"": 2, ""mimeType"": ""image/png"" }},
				{{ ""bufferView"": 1, ""mimeType"": ""image/png"" }},
				{{ ""bufferView"": 3, ""mimeType"": ""image/png"" }} ],
			""textures"": [{{ ""source"": 0 }}, {{ ""source"": 1 }}, {{ ""source"": 2 }}, {{ ""source"": 3 }}],
			""materials"": [
				{{ ""name"": ""m0"", ""pbrMetallicRoughness"": {{ ""baseColorTexture"": {{ ""index"": 0 }} }} }},
				{{ ""name"": ""m1"", ""pbrMetallicRoughness"": {{ ""baseColorTexture"": {{ ""index"": 1 }} }} }},
				{{ ""name"": ""m2"", ""pbrMetallicRoughness"": {{ ""baseColorTexture"": {{ ""index"": 2 }} }} }},
				{{ ""name"": ""m3"", ""pbrMetallicRoughness"": {{ ""baseColorTexture"": {{ ""index"": 3 }} }} }} ],
			""meshes"": [{{ ""primitives"": [
				{{ ""attributes"": {{ ""POSITION"": 0 }}, ""material"": 0 }},
				{{ ""attributes"": {{ ""POSITION"": 0 }}, ""material"": 1 }},
				{{ ""attributes"": {{ ""POSITION"": 0 }}, ""material"": 2 }},
				{{ ""attributes"": {{ ""POSITION"": 0 }}, ""material"": 3 }} ] }}]
		}}";

		Model model = Model.FromMemory("image_share.gltf", Encoding.UTF8.GetBytes(json));
		if (model == null) return false;

		Dictionary<string, string> texByMat = new Dictionary<string, string>();
		foreach (ModelNode node in model.Visuals)
		{
			Material mat = node.Material;
			texByMat[mat.Id] = mat.GetTexture("diffuse")?.Id;
			Log.Info($"{mat.Id} -> {texByMat[mat.Id]}");
		}

		string TexOf(string mat) => texByMat.TryGetValue($"image_share.gltf/mat/{mat}", out string id) ? id : null;
		return texByMat.Count == 4
			&& TexOf("m0") == "image_share.gltf/tex/0"
			&& TexOf("m1") == "image_share.gltf/tex/0"
			&& TexOf("m2") == "image_share.gltf/tex/0"
			&& TexOf("m3") == "image_share.gltf/tex/3";
	}

	public void Initialize() => Tests.Test(SharedRangesShareTex);
	public void Shutdown  () { }
	public void Step      () { }
}
