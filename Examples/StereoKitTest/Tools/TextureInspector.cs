// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

using System;

namespace StereoKit.Framework
{
	public class TextureInspector : IStepper
	{
		static TextureInspector _inst;

		Tex      tex;
		Material preview = new Material("Shaders/tex_preview.hlsl");
		float    mip;
		float    layer;
		float    exposure = 1;
		int      channel; // 0 is RGB, 1-4 are R, G, B, A on their own

		public Pose pose;

		public bool Enabled => true;

		public static void Show(Tex tex)
		{
			if (_inst == null)
				_inst = SK.AddStepper<TextureInspector>();
			if (_inst.tex == null) _inst.pose = UI.PopupPose();
			_inst.tex   = tex;
			_inst.mip   = 0;
			_inst.layer = 0;
			_inst.preview.SetTexture("source", tex);
		}

		public static void Hide() => _inst?.Close();

		// The preview material would otherwise keep the texture alive
		void Close()
		{
			tex = null;
			preview.SetTexture("source", Tex.White);
		}

		public bool Initialize() => true;
		public void Shutdown() { if (_inst == this) _inst = null; }

		public void Step()
		{
			if (tex == null) return;

			UI.WindowBegin("Texture Inspector", ref pose, new Vec2(56 * U.cm, 0));
			UI.LayoutPushCut(UICut.Left, 0.24f);
				DrawPreview();
				DrawViewControls();
			UI.LayoutPop();

			DrawInfo();
			if (tex != null)
			{
				UI.HSeparator();
				DrawSampler();
			}
			UI.WindowEnd();
		}

		void DrawPreview()
		{
			if ((tex.Type & (TexType.Volume | TexType.Zbuffer)) != 0)
			{
				UI.Label("No preview for 3D or zbuffer textures", new Vec2(UI.LayoutRemaining.x, 0));
				return;
			}

			float  size   = UI.LayoutRemaining.x;
			float  aspect = tex.Height > 0 ? tex.Width / (float)tex.Height : 1;
			Vec2   dims   = aspect >= 1 ? V.XY(size, size / aspect) : V.XY(size * aspect, size);
			Bounds b      = UI.LayoutReserve(V.XY(size, dims.y));
			b.center.z -= 0.002f;

			preview["mip"     ] = mip;
			preview["layer"   ] = layer;
			preview["exposure"] = exposure;
			preview["channels"] = channel == 0 ? new Vec4(1, 1, 1, 0) : ChannelMask(channel);
			Mesh.Quad.Draw(preview, Matrix.TS(b.center, V.XYZ(dims.x, dims.y, 1)));
		}

		static Vec4 ChannelMask(int channel) => new Vec4(
			channel == 1 ? 1 : 0,
			channel == 2 ? 1 : 0,
			channel == 3 ? 1 : 0,
			channel == 4 ? 1 : 0);

		void DrawViewControls()
		{
			float width = UI.LayoutRemaining.x;
			Vec2  label = new Vec2(width * 0.3f, UI.LineHeight);

			int mipCount = Math.Max(1, tex.Mips);
			if (mipCount > 1)
			{
				UI.Label($"Mip {(int)mip}", label); UI.SameLine();
				UI.HSlider("mip", ref mip, 0, mipCount - 1, 1);
			}

			int layers = Math.Max(1, tex.ArrayCount);
			if (layers > 1)
			{
				bool cube = (tex.Type & TexType.Cubemap) != 0;
				UI.Label(cube ? $"Face {CubeFace((int)layer)}" : $"Layer {(int)layer}", label); UI.SameLine();
				UI.HSlider("layer", ref layer, 0, layers - 1, 1);
			}

			UI.Label($"Exp {exposure:0.0}", label); UI.SameLine();
			UI.HSlider("exposure", ref exposure, 0, 8, 0);

			Vec2 radio = new Vec2((width - UI.Settings.gutter * 4) / 5, 0);
			for (int i = 0; i < channelNames.Length; i++)
			{
				if (i > 0) UI.SameLine();
				if (Choice(channelNames[i], channel == i, radio)) channel = i;
			}
		}

		static readonly string[] channelNames = { "RGB", "R", "G", "B", "A" };

		static string CubeFace(int face) => face switch {
			0 => "+X", 1 => "-X", 2 => "+Y", 3 => "-Y", 4 => "+Z", 5 => "-Z", _ => face.ToString() };

		void DrawInfo()
		{
			UI.PanelBegin();
			UI.Label(tex.Id ?? "(no id)", new Vec2(UI.LayoutRemaining.x - UI.LineHeight - UI.Settings.gutter, 0));
			UI.PanelEnd();
			UI.SameLine();
			if (UI.ButtonImg("close", Sprite.Close, UIBtnLayout.CenterNoText, V.XX(UI.LineHeight)))
			{
				Close();
				return;
			}

			float width = UI.LayoutRemaining.x - UI.Settings.gutter;
			Vec2  size1 = new Vec2(width * 0.4f, UI.LineHeight);
			Vec2  size2 = new Vec2(width * 0.6f, UI.LineHeight);

			string dims = $"{tex.Width}x{tex.Height}";
			if (tex.Depth      > 1) dims += $"x{tex.Depth}";
			if (tex.ArrayCount > 1) dims += $", {tex.ArrayCount} layers";

			UI.Label("Size:",         size1); UI.SameLine(); UI.Label(dims,                       size2);
			UI.Label("Mips:",         size1); UI.SameLine(); UI.Label(tex.Mips.ToString(),        size2);
			UI.Label("Type:",         size1); UI.SameLine(); UI.Label(tex.Type.ToString(),        size2);
			UI.Label("Format:",       size1); UI.SameLine(); UI.Label(tex.Format.ToString(),      size2);
			UI.Label("Input Format:", size1); UI.SameLine(); UI.Label(tex.InputFormat.ToString(), size2);
			UI.Label("State:",        size1); UI.SameLine(); UI.Label(tex.AssetState.ToString(),  size2);

			long gpuBytes   = GpuBytes(tex.Format);
			long inputBytes = GpuBytes(tex.InputFormat);
			UI.Label("GPU Memory:", size1); UI.SameLine(); UI.Label(gpuBytes > 0 ? Megabytes(gpuBytes) : "unknown", size2);
			if (gpuBytes > 0 && inputBytes > gpuBytes)
			{
				UI.Label("Uncompressed:", size1); UI.SameLine();
				UI.Label($"{Megabytes(inputBytes)}, {inputBytes / (float)gpuBytes:0.0}x", size2);
			}
		}

		void DrawSampler()
		{
			float width = UI.LayoutRemaining.x - UI.Settings.gutter;
			Vec2  size1 = new Vec2(width * 0.4f, UI.LineHeight);
			Vec2  third = new Vec2((width * 0.6f - UI.Settings.gutter * 2) / 3, 0);

			TexSample sample = tex.SampleMode;
			UI.Label("Sample:", size1);
			UI.SameLine(); if (Choice("Linear", sample == TexSample.Linear,      third)) tex.SampleMode = TexSample.Linear;
			UI.SameLine(); if (Choice("Point",  sample == TexSample.Point,       third)) tex.SampleMode = TexSample.Point;
			UI.SameLine(); if (Choice("Aniso",  sample == TexSample.Anisotropic, third)) tex.SampleMode = TexSample.Anisotropic;

			TexAddress address = tex.AddressMode;
			UI.Label("Address:", size1);
			UI.SameLine(); if (Choice("Wrap",   address == TexAddress.Wrap,   third)) tex.AddressMode = TexAddress.Wrap;
			UI.SameLine(); if (Choice("Clamp",  address == TexAddress.Clamp,  third)) tex.AddressMode = TexAddress.Clamp;
			UI.SameLine(); if (Choice("Mirror", address == TexAddress.Mirror, third)) tex.AddressMode = TexAddress.Mirror;
		}

		static bool Choice(string text, bool active, Vec2 size)
			=> UI.Radio(text, active, null, null, UIBtnLayout.None, size);

		static string Megabytes(long bytes) => bytes >= 1024 * 1024
			? $"{bytes / (1024.0 * 1024.0):0.00} MB"
			: $"{bytes / 1024.0:0.0} KB";

		// Every mip and layer of this texture, stored in format. Mips smaller
		// than a block still take a whole block.
		long GpuBytes(TexFormat format)
		{
			(int blockW, int blockH, int blockBytes) = BlockInfo(format);
			if (blockBytes == 0) return 0;

			long total  = 0;
			int  layers = Math.Max(1, tex.ArrayCount);
			for (int m = 0; m < Math.Max(1, tex.Mips); m++)
			{
				int w = Math.Max(1, tex.Width  >> m);
				int h = Math.Max(1, tex.Height >> m);
				int d = Math.Max(1, tex.Depth  >> m);
				total += (long)((w + blockW - 1) / blockW) * ((h + blockH - 1) / blockH) * blockBytes * d;
			}
			return total * layers;
		}

		static (int w, int h, int bytes) BlockInfo(TexFormat format) => format switch {
			TexFormat.R8 or TexFormat.R8sn or TexFormat.R8ui or TexFormat.R8si or TexFormat.R8Srgb => (1, 1, 1),
			TexFormat.R8g8 or TexFormat.R16un or TexFormat.R16sn or TexFormat.R16ui or TexFormat.R16si or TexFormat.R16f or TexFormat.Depth16 => (1, 1, 2),
			TexFormat.Rgba32Srgb or TexFormat.Rgba32Linear or TexFormat.Bgra32Srgb or TexFormat.Bgra32Linear or TexFormat.Rg11b10 or TexFormat.Rgb10a2 or TexFormat.Rgb9e5
				or TexFormat.R32ui or TexFormat.R32si or TexFormat.R32f or TexFormat.Depth16s8 or TexFormat.Depth24s8 or TexFormat.Depth32 => (1, 1, 4),
			TexFormat.Rgba64un or TexFormat.Rgba64sn or TexFormat.Rgba64ui or TexFormat.Rgba64si or TexFormat.Rgba64f or TexFormat.Depth32s8 => (1, 1, 8),
			TexFormat.Rgba128 => (1, 1, 16),
			TexFormat.Bc1RgbSrgb or TexFormat.Bc1Rgb or TexFormat.Bc1RgbaSrgb or TexFormat.Bc1Rgba or TexFormat.Bc4R or TexFormat.Bc4Rsn
				or TexFormat.Etc1Rgb or TexFormat.Etc1RgbSrgb or TexFormat.Etc2R11 or TexFormat.AtcRgb
				or TexFormat.Pvrtc1RgbSrgb or TexFormat.Pvrtc1Rgb or TexFormat.Pvrtc1RgbaSrgb or TexFormat.Pvrtc1Rgba or TexFormat.Pvrtc2RgbaSrgb or TexFormat.Pvrtc2Rgba => (4, 4, 8),
			TexFormat.Bc2RgbaSrgb or TexFormat.Bc2Rgba or TexFormat.Bc3RgbaSrgb or TexFormat.Bc3Rgba or TexFormat.Bc5Rg or TexFormat.Bc5Rgsn
				or TexFormat.Bc6hRgbuf or TexFormat.Bc6hRgbf or TexFormat.Bc7RgbaSrgb or TexFormat.Bc7Rgba
				or TexFormat.Etc2RgbaSrgb or TexFormat.Etc2Rgba or TexFormat.Etc2Rg11 or TexFormat.AtcRgba
				or TexFormat.Astc4x4RgbaSrgb or TexFormat.Astc4x4Rgba => (4, 4, 16),
			TexFormat.Astc6x6RgbaSrgb or TexFormat.Astc6x6Rgba => (6, 6, 16),
			TexFormat.Astc8x8RgbaHdr => (8, 8, 16),
			_ => (1, 1, 0),
		};
	}
}
