using StereoKit;

class TestTextureSingleChannel : ITest
{
	Material singleChannelTex = Default.Material.Copy();

	bool MakeTexture()
	{
		Tex r8  = new Tex(TexType.ImageNomips, TexFormat.R8);
		Tex r16 = new Tex(TexType.ImageNomips, TexFormat.R16);
		Tex r32 = new Tex(TexType.ImageNomips, TexFormat.R32);

		if (r8 == null || r16 == null || r32 == null)
			return false;

		int s = 8;
		byte  [] r8Data  = new byte  [s*s];
		ushort[] r16Data = new ushort[s*s];
		float [] r32Data = new float [s*s];
		for (int i = 0; i < s*s; i++)
		{
			float y = (float)(i/s) / s;
			r8Data [i] = (byte)(y*byte.MaxValue);
			r16Data[i] = (ushort)(y*ushort.MaxValue);
			r32Data[i] = y;
		}
		r8 .SetColors(s, s, r8Data);
		r16.SetColors(s, s, r16Data);
		r32.SetColors(s, s, r32Data);

		r8.AddressMode  = TexAddress.Clamp;
		r16.AddressMode = TexAddress.Clamp;
		r32.AddressMode = TexAddress.Clamp;

		singleChannelTex[MatParamName.DiffuseTex] = r8;
		return true;
	}

	public void Initialize()
	{
		Tests.Test(MakeTexture);
	}

	public void Shutdown() { }
	public void Step() 
	{
		Default.MeshCube.Draw(singleChannelTex, Matrix.TS(new Vec3(0,0,0.75f), 0.5f));
		Tests.Screenshot("Tests/SingleChannelTextureR8.jpg", 600, 600, Vec3.Zero, -Vec3.Forward);
	}
}
