// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

// The video player on Android is a source of OES External textures, and the
// Android.Media.MediaPlayer is integrated in C#'s core APIs. MediaPlayer
// unfortunately doesn't seem to expose the audio stream, so it won't let us do
// spatialized audio for the videos. This makes MediaPlayer fine for testing
// external textures, but not really robust enough for general video playing.

#if ANDROID

using Android.Graphics;
using Android.Media;
using Android.Views;
using Android.Opengl;
using StereoKit;
using System;

class TestVideo : ITest
{
	struct Video
	{
		public VideoTexture vtex;
		public Material     mat;
		public Pose         pose;
	}

	Video[] videos;
	bool    enabled;

	public void Initialize()
	{
		// This test uses OpenGL ES external textures, not supported on Vulkan
		if (Backend.Graphics != BackendGraphics.OpenGLES_EGL)
		{
			Log.Info("TestVideo: Skipping, requires OpenGL ES backend");
			return;
		}
		enabled = true;

		videos = new Video[] {
			new Video() {
				vtex = new VideoTexture("https://download.blender.org/peach/bigbuckbunny_movies/BigBuckBunny_320x180.mp4"),
				mat  = new Material("Shaders/unlit_external.hlsl"),
				pose = new Pose(-1,0,-1, Quat.LookDir(0,0,1))
			},
			new Video() {
				vtex = new VideoTexture("https://download.blender.org/durian/trailer/sintel_trailer-480p.mp4"),
				mat  = new Material("Shaders/unlit_external.hlsl"),
				pose = new Pose(1,0,-1, Quat.LookDir(0,0,1))
			},
		};

		for (int i = 0; i < videos.Length; i++)
		{
			videos[i].mat[MatParamName.DiffuseTex] = videos[i].vtex.Texture;
			videos[i].vtex.VideoReady += (o,s) => ((VideoTexture)o).Play();
			videos[i].vtex.VideoError += (o,m) => Log.Err(m);
		}
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		if (!enabled) return;

		for (int i = 0; i < videos.Length; i++)
		{
			Video v = videos[i];
			v.vtex.UpdateTexture();
			StereoKit.Mesh.Quad.Draw(v.mat, v.pose.ToMatrix(V.XYZ(v.vtex.Width/(float)v.vtex.Height, 1, 1)));
		}
	}
}

public class VideoTexture : Java.Lang.Object, SurfaceTexture.IOnFrameAvailableListener
{
	private MediaPlayer    mediaPlayer;
	private SurfaceTexture surfaceTexture;
	private Surface        surface;
	private int            externalTextureId;
	private Tex            stereoKitTex;
	private volatile bool  frameAvailable  = false;

	public int    Width     => mediaPlayer?.VideoWidth  ?? 1;
	public int    Height    => mediaPlayer?.VideoHeight ?? 1;
	public bool   IsPlaying => mediaPlayer?.IsPlaying   ?? false;
	public Tex    Texture   => stereoKitTex;

	// Events
	public event EventHandler         VideoReady;
	public event EventHandler<string> VideoError;

	public VideoTexture(string videoPath)
	{
		// Generate OpenGL external texture ID
		int[] textureIds = new int[1];
		GLES20.GlGenTextures(1, textureIds, 0);
		externalTextureId = textureIds[0];

		// Set it up
		GLES20.GlBindTexture  (GLES11Ext.GlTextureExternalOes, externalTextureId);
		GLES20.GlTexParameteri(GLES11Ext.GlTextureExternalOes, GLES20.GlTextureMinFilter, GLES20.GlLinear);
		GLES20.GlTexParameteri(GLES11Ext.GlTextureExternalOes, GLES20.GlTextureMagFilter, GLES20.GlLinear);
		GLES20.GlTexParameteri(GLES11Ext.GlTextureExternalOes, GLES20.GlTextureWrapS,     GLES20.GlClampToEdge);
		GLES20.GlTexParameteri(GLES11Ext.GlTextureExternalOes, GLES20.GlTextureWrapT,     GLES20.GlClampToEdge);

		// Create SurfaceTexture and Surface
		surfaceTexture = new SurfaceTexture(externalTextureId);
		surfaceTexture.SetOnFrameAvailableListener(this);
		surface = new Surface(surfaceTexture);

		// Setup media player
		mediaPlayer = new MediaPlayer();
		try
		{
			mediaPlayer.SetDataSource(videoPath);
			mediaPlayer.SetSurface   (surface);
			mediaPlayer.Looping   = true;
			mediaPlayer.Prepared += (sender, args) => VideoReady?.Invoke(this, EventArgs.Empty);
			mediaPlayer.Error    += (sender, args) => VideoError?.Invoke(this, $"MediaPlayer error: {args.What}");
			mediaPlayer.PrepareAsync();
		}
		catch (Exception e) { VideoError?.Invoke(this, $"Error setting up media player: {e.Message}"); }

		stereoKitTex = new Tex(TexType.ImageNomips);
		stereoKitTex.SetNativeSurface(externalTextureId, TexType.ImageNomips, 0, 1, 1);
	}
	
	~VideoTexture()
	{
		mediaPlayer   ?.Release();
		surface       ?.Release();
		surfaceTexture?.Release();

		if (externalTextureId != 0)
			GLES20.GlDeleteTextures(1, new int[] { externalTextureId }, 0);
	}

	public void OnFrameAvailable(SurfaceTexture surface)
	{
		frameAvailable = true;
	}

	public bool UpdateTexture()
	{
		if (!frameAvailable) return false;

		surfaceTexture.UpdateTexImage();
		frameAvailable = false;
		return true;
	}

	// Playback controls
	public void Play() => mediaPlayer?.Start();
	public void Pause() => mediaPlayer?.Pause();
	public void SeekTo(int milliseconds) => mediaPlayer?.SeekTo(milliseconds);
	public void SetVolume(float volume) => mediaPlayer?.SetVolume(volume, volume);
}
#endif