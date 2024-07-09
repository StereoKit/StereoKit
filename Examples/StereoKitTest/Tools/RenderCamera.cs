using System;
using System.IO;

namespace StereoKit.Framework
{
	public class RenderCamera : IStepper
	{
		public bool Enabled => true;

		public int Width     { get; private set; }
		public int Height    { get; private set; }
		public int FrameRate { get; private set; }

		public float Fov => 10 + Math.Max(0, Math.Min(1, (Vec3.Distance(from.position, at.position) - 0.1f) / 0.2f)) * 110;
		public Vec3  CamAt  => at.position + (at.position - from.position).Normalized * 0.06f;
		public Vec3  CamDir => (at.position - from.position).Normalized;

		public string folder  = "Video";
		public Pose   from;
		public Pose   at;
		public float  damping = 8;

		private int      _frameIndex;
		private float    _frameTime;
		private Tex      _frameSurface;
		private Material _frameMaterial;
		private bool     _recording = false;
		private Pose     _renderFrom;
		private bool     _previewing = false;

		public RenderCamera(Pose startAt, int width = 500, int height = 500, int framerate = 12)
		{
			Width     = width;
			Height    = height;
			FrameRate = framerate;

			at   = startAt;
			from = new Pose(at.position+V.XYZ(0, 0, 0.3f)*at.orientation, at.orientation);
			_renderFrom = at;
		}

		public bool Initialize()
		{
			_frameSurface = Tex.RenderTarget(Width, Height, 1, TexFormat.Rgba32, TexFormat.Depth32);
			_frameMaterial = Default.MaterialUnlit.Copy();
			_frameMaterial[MatParamName.DiffuseTex] = _frameSurface;
			_frameMaterial.FaceCull = Cull.None;
			return true;
		}

		public void Shutdown()
		{
		}

		public void Step()
		{
			UI.PushId("RenderCameraWidget");
			UI.Handle     ("from", ref from, new Bounds(Vec3.One*0.02f), true);
			UI.HandleBegin("at",   ref at,   new Bounds(Vec3.One*0.02f), true);
			UI.ToggleAt("On", ref _previewing, new Vec3(4, -2, 0) * U.cm, new Vec2(8 * U.cm, UI.LineHeight));
			if (_previewing && UI.ToggleAt("Record", ref _recording, new Vec3(4, -6, 0)*U.cm, new Vec2(8*U.cm, UI.LineHeight))) {
				_frameTime  = Time.StepUnscaledf;
				_frameIndex = 0;
			}
			UI.HandleEnd();
			UI.PopId();

			float fov        = Fov;
			Vec3  previewAt  = at.position + at.orientation * Vec3.Up * 0.06f;
			Vec3  renderFrom = CamAt;
			_renderFrom = Pose.Lerp(_renderFrom, new Pose(renderFrom, Quat.LookDir(at.position - from.position)), Time.Stepf * damping);

			Lines.Add(from.position, at.position, Color.White, 0.005f);
			from.orientation = at.orientation = Quat.LookDir(from.position-at.position);

			
			if (_previewing)
			{
				Hierarchy.Push(Matrix.TR(previewAt, Quat.LookAt(previewAt, Input.Head.position)));
				Default.MeshQuad.Draw(_frameMaterial, Matrix.S(V.XYZ(0.08f * ((float)Width / Height), 0.08f, 1)));
				Text.Add(""+(int)fov, Matrix.TS(-0.03f,0,0, 0.5f), TextAlign.CenterLeft);
				Hierarchy.Pop();

				Matrix glFix = Backend.Graphics == BackendGraphics.D3D11 ? Matrix.Identity : Matrix.R(0,0,180);
				Renderer.RenderTo(_frameSurface,
					glFix * _renderFrom.ToMatrix(),
					Matrix.Perspective(fov, (float)Width/Height, 0.01f, 100));
			}
			if (_recording)
				SaveFrame(FrameRate);
		}

		Color32[] buffer = null;
		void SaveFrame(int framerate)
		{
			float rateTime = 1.0f / framerate;
			if (_frameTime + rateTime < Time.TotalUnscaledf)
			{
				_frameTime = Time.TotalUnscaledf;
				_frameSurface.GetColorData(ref buffer);

				Directory.CreateDirectory(folder);
				Stream writer = new FileStream($"{folder}/image{_frameIndex:D4}.bmp", FileMode.Create);
				WriteBitmap(writer, _frameSurface.Width, _frameSurface.Height, buffer);
				writer.Close();
				_frameIndex += 1;
			}
		}

		static void WriteBitmap(Stream stream, int width, int height, Color32[] imageData)
		{
			using (BinaryWriter bw = new BinaryWriter(stream))
			{
				// define the bitmap file header
				bw.Write((UInt16)0x4D42);                               // bfType;
				bw.Write((UInt32)(14 + 40 + (width * height * 4)));     // bfSize;
				bw.Write((UInt16)0);                                    // bfReserved1;
				bw.Write((UInt16)0);                                    // bfReserved2;
				bw.Write((UInt32)14 + 40);                              // bfOffBits;

				// define the bitmap information header
				bw.Write((UInt32)40);                                   // biSize;
				bw.Write((Int32)width);                                 // biWidth;
				bw.Write((Int32)height);                                // biHeight;
				bw.Write((UInt16)1);                                    // biPlanes;
				bw.Write((UInt16)32);                                   // biBitCount;
				bw.Write((UInt32)0);                                    // biCompression;
				bw.Write((UInt32)(width * height * 4));                 // biSizeImage;
				bw.Write((Int32)0);                                     // biXPelsPerMeter;
				bw.Write((Int32)0);                                     // biYPelsPerMeter;
				bw.Write((UInt32)0);                                    // biClrUsed;
				bw.Write((UInt32)0);                                    // biClrImportant;

				// switch the image data from RGB to BGR
				for (int y = 0; y < height; y++)
				{
					for (int x = 0; x < width; x++)
					{
						int i = x + ((height - 1) - y) * width;
						bw.Write(imageData[i].b);
						bw.Write(imageData[i].g);
						bw.Write(imageData[i].r);
						bw.Write(imageData[i].a);
					}
				}
			}
		}
	}
}
