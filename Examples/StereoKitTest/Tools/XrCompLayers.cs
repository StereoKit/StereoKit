using System;
using System.Runtime.InteropServices;

namespace StereoKit.Framework
{
	class XrCompLayers : IStepper
	{
		static XrCompLayers _inst;

		bool _androidSwapchainAvailable;
		public bool Enabled { get; private set; }

		public XrCompLayers()
		{
			_inst = this;
			if (SK.IsInitialized)
				Log.Err("CompositionLayers must be Setup before StereoKit is initialized!");

			Backend.OpenXR.RequestExt("XR_KHR_android_surface_swapchain");
		}

		public bool Initialize()
		{
			_androidSwapchainAvailable = Backend.OpenXR.ExtEnabled("XR_KHR_android_surface_swapchain");
			Enabled =
				Backend.XRType == BackendXRType.OpenXR &&
				LoadBindings();

			return true;
		}

		public void Shutdown() { }
		public void Step() { }

		static long ToNativeFormat(TexFormat format)
		{
			long nativeFormat;
			if (Backend.Graphics == BackendGraphics.D3D11)
			{
				switch (format)
				{
					case TexFormat.Rgba32:       nativeFormat = 29; break;
					case TexFormat.Rgba32Linear: nativeFormat = 28; break;
					case TexFormat.Bgra32:       nativeFormat = 91; break;
					case TexFormat.Bgra32Linear: nativeFormat = 87; break;
					case TexFormat.Rgb10a2:      nativeFormat = 24; break;
					case TexFormat.Rg11b10:      nativeFormat = 26; break;
					default: throw new NotImplementedException();
				}
			}
			else if (Backend.Graphics == BackendGraphics.OpenGLES_EGL)
			{
				switch (format)
				{
					case TexFormat.Rgba32:       nativeFormat = 0x8C43; break;
					case TexFormat.Rgba32Linear: nativeFormat = 0x8058; break;
					case TexFormat.Rgb10a2:      nativeFormat = 0x8059; break;
					case TexFormat.Rg11b10:      nativeFormat = 0x8C3A; break;
					default: throw new NotImplementedException();
				}
			}
			else if (Backend.Graphics == BackendGraphics.Vulkan)
			{
				switch (format)
				{
					case TexFormat.Rgba32:       nativeFormat = 43;  break; // VK_FORMAT_R8G8B8A8_SRGB
					case TexFormat.Rgba32Linear: nativeFormat = 37;  break; // VK_FORMAT_R8G8B8A8_UNORM
					case TexFormat.Bgra32:       nativeFormat = 50;  break; // VK_FORMAT_B8G8R8A8_SRGB
					case TexFormat.Bgra32Linear: nativeFormat = 44;  break; // VK_FORMAT_B8G8R8A8_UNORM
					case TexFormat.Rgb10a2:      nativeFormat = 64;  break; // VK_FORMAT_A2B10G10R10_UNORM_PACK32
					case TexFormat.Rg11b10:      nativeFormat = 122; break; // VK_FORMAT_B10G11R11_UFLOAT_PACK32
					default: throw new NotImplementedException();
				}
			}
			else throw new NotImplementedException();
			return nativeFormat;
		}

		public static void SubmitQuadLayer(Pose worldPose, Vec2 size, XrSwapchain swapchain, Rect swapchainRect, uint swapchainArrayIndex, int compositionSortOrder, XrEyeVisibility visibility = XrEyeVisibility.BOTH, ulong? xrSpace = null)
		{
			if (_inst.Enabled == false) return;

			XrCompositionLayerQuad quadLayer = new XrCompositionLayerQuad();
			quadLayer.type          = XrStructureType.COMPOSITION_LAYER_QUAD;
			quadLayer.space         = xrSpace ?? Backend.OpenXR.Space;
			quadLayer.eyeVisibility = visibility;
			quadLayer.size          = new XrExtent2Df { width = size.x, height = size.y };
			quadLayer.pose          = new XrPosef { position = worldPose.position, orientation = worldPose.orientation * Quat.FromAngles(0,180,0) }; // OpenXR orients layers BACKWARDS!!
			quadLayer.layerFlags    = XrCompositionLayerFlags.BLEND_TEXTURE_SOURCE_ALPHA_BIT;

			quadLayer.subImage = new XrSwapchainSubImage {
				swapchain = swapchain,
				imageRect = new XrRect2Di {
					offset = new XrOffset2Di { x     = (int)swapchainRect.x,     y      = (int)swapchainRect.y },
					extent = new XrExtent2Di { width = (int)swapchainRect.width, height = (int)swapchainRect.height } },
				imageArrayIndex = swapchainArrayIndex

			};
			Backend.OpenXR.AddCompositionLayer(quadLayer, compositionSortOrder);
		}

		public static bool TryMakeAndroidSwapchain(int width, int height, UsageFlags usage, out XrSwapchain swapchain, out IntPtr surfaceJObject)
		{
			swapchain      = default;
			surfaceJObject = default;
			if (!_inst._androidSwapchainAvailable || _inst.Enabled == false) return false;

			XrSwapchainCreateInfo info = new XrSwapchainCreateInfo();
			info.type        = XrStructureType.SWAPCHAIN_CREATE_INFO;
			info.usageFlags  = usage;
			info.width       = (uint)width;
			info.height      = (uint)height;
			
			// Required by spec to be zero
			info.format      = 0;
			info.sampleCount = 0;
			info.mipCount    = 0;
			info.arraySize   = 0;
			info.faceCount   = 0;

			XrResult result = _inst.xrCreateSwapchainAndroidSurfaceKHR(Backend.OpenXR.Session, info, out swapchain, out surfaceJObject);
			return result == XrResult.Success;
		}

		public static bool TryMakeSwapchain(int width, int height, TexFormat format, UsageFlags usage, bool singleImage, out XrSwapchain swapchain)
		{
			swapchain = default;
			if (_inst.Enabled == false) return false;

			// Create the swapchain
			XrSwapchainCreateInfo info = new XrSwapchainCreateInfo();
			info.type        = XrStructureType.SWAPCHAIN_CREATE_INFO;
			info.format      = ToNativeFormat(format);
			info.usageFlags  = usage;
			info.width       = (uint)width;
			info.height      = (uint)height;
			info.sampleCount = 1;
			info.mipCount    = 1;
			info.arraySize   = 1;
			info.faceCount   = 1;
			if (singleImage)
				info.createFlags = XrSwapchainCreateFlags.STATIC_IMAGE_BIT;

			XrResult result = _inst.xrCreateSwapchain(Backend.OpenXR.Session, info, out swapchain);
			return result == XrResult.Success;
		}

		public static void DestroySwapchain(XrSwapchain swapchain)
		{
			if (_inst.Enabled == false) return;

			_inst.xrDestroySwapchain(swapchain);
		}

		#region OpenXR native bindings and types

		enum XrCompositionLayerFlags : UInt64
		{
			None = 0,
			CORRECT_CHROMATIC_ABERRATION_BIT = 0x00000001,
			BLEND_TEXTURE_SOURCE_ALPHA_BIT = 0x00000002,
			UNPREMULTIPLIED_ALPHA_BIT = 0x00000004,
		}

		enum XrSwapchainCreateFlags : UInt64
		{
			PROTECTED_CONTENT_BIT = 0x00000001,
			STATIC_IMAGE_BIT = 0x00000002,
		}

		[Flags]
		public enum UsageFlags : UInt64
		{
			COLOR_ATTACHMENT_BIT = 0x00000001,
			DEPTH_STENCIL_ATTACHMENT_BIT = 0x00000002,
			UNORDERED_ACCESS_BIT = 0x00000004,
			TRANSFER_SRC_BIT = 0x00000008,
			TRANSFER_DST_BIT = 0x00000010,
			SAMPLED_BIT = 0x00000020,
			MUTABLE_FORMAT_BIT = 0x00000040,
			INPUT_ATTACHMENT_BIT_MND = 0x00000080,
			INPUT_ATTACHMENT_BIT_KHR = 0x00000080,
		}

		enum XrResult : Int32
		{
			Success = 0,
		}

		public enum XrEyeVisibility : Int32
		{
			BOTH = 0,
			LEFT = 1,
			RIGHT = 2,
		}

		enum XrStructureType : UInt32
		{
			COMPOSITION_LAYER_QUAD = 36,
			SWAPCHAIN_CREATE_INFO = 9,
			SWAPCHAIN_IMAGE_ACQUIRE_INFO = 55,
			SWAPCHAIN_IMAGE_WAIT_INFO = 56,
			SWAPCHAIN_IMAGE_RELEASE_INFO = 57,
			SWAPCHAIN_IMAGE_OPENGL_ES_KHR = 1000024002,
			SWAPCHAIN_IMAGE_D3D11_KHR = 1000027001,
			SWAPCHAIN_IMAGE_VULKAN_KHR = 1000025001,
		}

#pragma warning disable 0169
		[StructLayout(LayoutKind.Sequential)] public struct XrSwapchain
		{
			ulong handle;
			public override string ToString() => handle.ToString();
		}
#pragma warning restore 0169

		[StructLayout(LayoutKind.Sequential)]
		struct XrPosef
		{
			public Quat orientation;
			public Vec3 position;
		}

		[StructLayout(LayoutKind.Sequential)]
		struct XrExtent2Df
		{
			public float width;
			public float height;
		}

		[StructLayout(LayoutKind.Sequential)]
		struct XrOffset2Di
		{
			public int x;
			public int y;
		}

		[StructLayout(LayoutKind.Sequential)]
		struct XrExtent2Di
		{
			public int width;
			public int height;
		}

		[StructLayout(LayoutKind.Sequential)]
		struct XrRect2Di
		{
			public XrOffset2Di offset;
			public XrExtent2Di extent;
		}

		[StructLayout(LayoutKind.Sequential)]
		struct XrSwapchainSubImage
		{
			public XrSwapchain swapchain;
			public XrRect2Di imageRect;
			public uint imageArrayIndex;
		}

		[StructLayout(LayoutKind.Sequential)]
		struct XrSwapchainCreateInfo
		{
			public XrStructureType type;
			public IntPtr next;
			public XrSwapchainCreateFlags createFlags;
			public UsageFlags usageFlags;
			public long format;
			public uint sampleCount;
			public uint width;
			public uint height;
			public uint faceCount;
			public uint arraySize;
			public uint mipCount;
		}

		[StructLayout(LayoutKind.Sequential)]
		struct XrSwapchainImageWaitInfo
		{
			public XrStructureType type;
			public IntPtr next;
			public long timeout;
		}

		[StructLayout(LayoutKind.Sequential)]
		struct XrSwapchainImageVulkanKHR
		{
			public XrStructureType type;
			public IntPtr next;
			public IntPtr VkImage;
		}

		[StructLayout(LayoutKind.Sequential)]
		struct XrSwapchainImageD3D11KHR
		{
			public XrStructureType type;
			public IntPtr next;
			public IntPtr ID3D11Texture2DTexture;
		}

		[StructLayout(LayoutKind.Sequential)]
		struct XrSwapchainImageOpenGLESKHR
		{
			public XrStructureType type;
			public IntPtr next;
			public uint image;
		}

		[StructLayout(LayoutKind.Sequential)]
		struct XrCompositionLayerQuad
		{
			public XrStructureType type;
			public IntPtr next;
			public XrCompositionLayerFlags layerFlags;
			public ulong space;
			public XrEyeVisibility eyeVisibility;
			public XrSwapchainSubImage subImage;
			public XrPosef pose;
			public XrExtent2Df size;
		}

		delegate XrResult del_xrCreateSwapchainAndroidSurfaceKHR(ulong session, in XrSwapchainCreateInfo createInfo, out XrSwapchain swapchain, out IntPtr surfaceJObject);
		delegate XrResult del_xrCreateSwapchain                 (ulong session, in XrSwapchainCreateInfo createInfo, out XrSwapchain swapchain);
		delegate XrResult del_xrDestroySwapchain        (XrSwapchain swapchain);
		delegate XrResult del_xrEnumerateSwapchainImages(XrSwapchain swapchain, uint imageCapacityInput, out uint imageCountOutput, IntPtr images);
		delegate XrResult del_xrAcquireSwapchainImage   (XrSwapchain swapchain, IntPtr acquireInfo, out uint index);
		delegate XrResult del_xrWaitSwapchainImage      (XrSwapchain swapchain, in XrSwapchainImageWaitInfo waitInfo);
		delegate XrResult del_xrReleaseSwapchainImage   (XrSwapchain swapchain, IntPtr releaseInfo);

		del_xrCreateSwapchainAndroidSurfaceKHR xrCreateSwapchainAndroidSurfaceKHR;
		del_xrCreateSwapchain                  xrCreateSwapchain;
		del_xrDestroySwapchain                 xrDestroySwapchain;
		del_xrEnumerateSwapchainImages         xrEnumerateSwapchainImages;
		del_xrAcquireSwapchainImage            xrAcquireSwapchainImage;
		del_xrWaitSwapchainImage               xrWaitSwapchainImage;
		del_xrReleaseSwapchainImage            xrReleaseSwapchainImage;

		bool LoadBindings()
		{
			// These are extension functions that may or may not be present.
			if (_androidSwapchainAvailable)
				xrCreateSwapchainAndroidSurfaceKHR = Backend.OpenXR.GetFunction<del_xrCreateSwapchainAndroidSurfaceKHR>("xrCreateSwapchainAndroidSurfaceKHR");

			xrCreateSwapchain          = Backend.OpenXR.GetFunction<del_xrCreateSwapchain         >("xrCreateSwapchain");
			xrDestroySwapchain         = Backend.OpenXR.GetFunction<del_xrDestroySwapchain        >("xrDestroySwapchain");
			xrEnumerateSwapchainImages = Backend.OpenXR.GetFunction<del_xrEnumerateSwapchainImages>("xrEnumerateSwapchainImages");
			xrAcquireSwapchainImage    = Backend.OpenXR.GetFunction<del_xrAcquireSwapchainImage   >("xrAcquireSwapchainImage");
			xrWaitSwapchainImage       = Backend.OpenXR.GetFunction<del_xrWaitSwapchainImage      >("xrWaitSwapchainImage");
			xrReleaseSwapchainImage    = Backend.OpenXR.GetFunction<del_xrReleaseSwapchainImage   >("xrReleaseSwapchainImage");

			return
				xrCreateSwapchain          != null &&
				xrDestroySwapchain         != null &&
				xrEnumerateSwapchainImages != null &&
				xrAcquireSwapchainImage    != null &&
				xrWaitSwapchainImage       != null &&
				xrReleaseSwapchainImage    != null;
		}

		#endregion

		#region Convenience wrapper for XRSwapchain
		public class Swapchain
		{
			public XrSwapchain handle;
			public int width;
			public int height;
			public Tex RenderTarget { get => _images[_acquired]; }

			uint  _acquired;
			Tex[] _images;

			public Swapchain(XrSwapchain handle, TexFormat format, int width, int height)
			{
				this.handle = handle;
				this.width  = width;
				this.height = height;
				if (_inst.xrEnumerateSwapchainImages(handle, 0, out uint imageCount, IntPtr.Zero) != XrResult.Success) throw new Exception();
				_images = new Tex[imageCount];

				GCHandle mem;
				if (Backend.Graphics == BackendGraphics.D3D11)
				{
					XrSwapchainImageD3D11KHR[] d3dImages = new XrSwapchainImageD3D11KHR[imageCount];
					for (int i = 0; i < d3dImages.Length; i++) d3dImages[i].type = XrStructureType.SWAPCHAIN_IMAGE_D3D11_KHR;
					mem = GCHandle.Alloc(d3dImages, GCHandleType.Pinned);
					if (_inst.xrEnumerateSwapchainImages(handle, imageCount, out uint finalCount, mem.AddrOfPinnedObject()) != XrResult.Success) throw new Exception();

					for (uint i = 0; i< d3dImages.Length; i++)
					{
						_images[i] = new Tex(TexType.Rendertarget, format);
						_images[i].SetNativeSurface(d3dImages[i].ID3D11Texture2DTexture, TexType.Rendertarget, ToNativeFormat(format), width, height);
					}
				}
				else if (Backend.Graphics == BackendGraphics.OpenGLES_EGL)
				{
					XrSwapchainImageOpenGLESKHR[] glesImages = new XrSwapchainImageOpenGLESKHR[imageCount];
					for (int i = 0; i < glesImages.Length; i++) glesImages[i].type = XrStructureType.SWAPCHAIN_IMAGE_OPENGL_ES_KHR;
					mem = GCHandle.Alloc(glesImages, GCHandleType.Pinned);
					if (_inst.xrEnumerateSwapchainImages(handle, imageCount, out uint finalCount, mem.AddrOfPinnedObject()) != XrResult.Success) throw new Exception();

					for (uint i = 0; i < glesImages.Length; i++)
					{
						_images[i] = new Tex(TexType.Rendertarget, format);
						_images[i].SetNativeSurface((IntPtr)glesImages[i].image, TexType.Rendertarget, ToNativeFormat(format), width, height);
					}
				}
				else if (Backend.Graphics == BackendGraphics.Vulkan)
				{
					XrSwapchainImageVulkanKHR[] vkImages = new XrSwapchainImageVulkanKHR[imageCount];
					for (int i = 0; i < vkImages.Length; i++) vkImages[i].type = XrStructureType.SWAPCHAIN_IMAGE_VULKAN_KHR;
					mem = GCHandle.Alloc(vkImages, GCHandleType.Pinned);
					if (_inst.xrEnumerateSwapchainImages(handle, imageCount, out uint finalCount, mem.AddrOfPinnedObject()) != XrResult.Success) throw new Exception();

					for (uint i = 0; i < vkImages.Length; i++)
					{
						_images[i] = new Tex(TexType.Rendertarget, format);
						_images[i].SetNativeSurface(vkImages[i].VkImage, TexType.Rendertarget, ToNativeFormat(format), width, height);
					}
				}
				else throw new NotImplementedException();

				mem.Free();
			}

			public bool Acquire(long timeoutNs = 0x7fffffffffffffff)
			{
				if (_inst.xrAcquireSwapchainImage(handle, IntPtr.Zero, out _acquired ) != XrResult.Success)
					return false;
				if (_inst.xrWaitSwapchainImage(handle, new XrSwapchainImageWaitInfo { type = XrStructureType.SWAPCHAIN_IMAGE_WAIT_INFO, timeout = timeoutNs }) != XrResult.Success)
					return false;

				return true;
			}

			public void Release()
				=> _inst.xrReleaseSwapchainImage(handle, IntPtr.Zero);
		}
		#endregion
	}
}