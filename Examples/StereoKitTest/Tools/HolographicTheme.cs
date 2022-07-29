using System;
using StereoKit;

namespace StereoKit.Framework
{
	static class HolographicTheme
	{
		public static void Apply()
		{
			UI.ColorScheme = new Color(0.137f, 0.054f, 0.386f);
			Vec3 b = new Color(0.137f, 0.054f, 0.386f).ToHSV();

			UI.Settings = new UISettings { padding = 0.01f, gutter = 0.005f, depth = 0.0075f };

			Shader   uiQuadrantShader   = Shader.FromFile("holographic_quadrant.hlsl");
			Material uiQuadrantMaterial = new Material(uiQuadrantShader);
			uiQuadrantMaterial[MatParamName.ColorTint] = Color.Hex(0x777ae8ff);// Color.HSV(b.x,b.y*0.6f,b.z*1.4f);
			Material uiGlassMaterial    = new Material(uiQuadrantShader);
			uiGlassMaterial.Transparency = Transparency.Blend;
			uiGlassMaterial.DepthWrite = false;
			uiGlassMaterial.QueueOffset = -20;
			uiGlassMaterial[MatParamName.ColorTint] = Color.White;

			Mesh quadrantCube = Mesh.GenerateCube(Vec3.One);
			UI.QuadrantSizeMesh(ref quadrantCube);

			Mesh backplate      = BackplateMesh(.01f, 6);
			Mesh backplateSmall = BackplateMesh(.004f, 6);
			Mesh glassMesh      = GlassButtonMesh(.01f, 6);
			UI.SetElementVisual(UIVisual.Default,     backplate,      uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.WindowHead,  backplate,      uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.WindowBody,  backplate,      uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.Separator,   quadrantCube,   uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.SliderLine,  backplateSmall, uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.SliderPinch, backplateSmall, uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.SliderPush,  backplateSmall, uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.Button,      glassMesh,      uiGlassMaterial);
			UI.SetElementVisual(UIVisual.Toggle,      glassMesh,      uiGlassMaterial);

			UI.SetThemeColor(UIColor.Primary,    Color.HSV(b));
			UI.SetThemeColor(UIColor.Background, Color.Hex(0x43499cff));//Color.HSV(b.x,b.y*0.8f,b.z));
			UI.SetThemeColor(UIColor.Common,     Color.Hex(0x5d5ed3ff));//Color.HSV(b.x,b.y*0.6f,b.z*1.2f));

			ApplyLighting();
		}

		public static void ApplyLighting()
		{
			var light = Renderer.SkyLight;
			light.Brightness(0.02f);
			Renderer.SkyTex = Tex.GenCubemap(light);
		}

		static Mesh BackplateMesh(float cornerRadius, int cornerResolution)
		{
			Mesh mesh = new Mesh();
	
			float radius = cornerRadius;

			int      subd       = cornerResolution * 4;
			uint     usubd      = (uint)subd;
			int      vert_count = 4 * (subd + 2);
			int      ind_count  = 4 * (cornerResolution * 12 + 6) + 12;
			Vertex[] verts      = new Vertex[vert_count];
			uint[]   inds       = new uint  [ind_count];

			uint ind = 0;
			Vec3 front = V.XYZ(0, 0, 0.5f);
			for (int c = 0; c < 4; c++)
			{
				float u = c == 0 || c == 3 ? 1 : -1;
				float v = c == 0 || c == 1 ? 1 : -1;
				Vec3 offset = V.XY0(-u*radius, -v*radius);

				uint vert = (uint)(c * (usubd + 2));
				verts[vert    ] = new Vertex(offset + front, V.XYZ(0, 0, 1), V.XY(u, v));
				verts[vert + 1] = new Vertex(offset - front, V.XYZ(0, 0,-1), V.XY(u, v));
				uint cornerFront = vert;
				uint cornerBack  = vert+1;

				for (uint i = 0; i < cornerResolution; i++)
				{
					float ang = (c*90 + (i/(float)(cornerResolution-1))*90) * Units.deg2rad;
					float x = SKMath.Cos(ang);
					float y = SKMath.Sin(ang);
					Vec3 normal  = V.XY0(x,y);
					Vec3 top_pos = normal*radius + V.XYZ(0,0,0.5f);
					Vec3 bot_pos = normal*radius - V.XYZ(0,0,0.5f);

					uint i4   = vert+2 + i * 4;
					uint i4_n = vert+2 + (i + 1) * 4;

					// strip around the edge first
					verts[i4    ] = new Vertex(offset+top_pos, normal, V.XY(u, v), new Color32(255, 255, 255, 0));
					verts[i4 + 1] = new Vertex(offset+bot_pos, normal, V.XY(u, v), new Color32(255, 255, 255, 0));
					// now for the front and back faces
					verts[i4 + 2] = new Vertex(offset+top_pos, V.XYZ(0, 0,  1), V.XY(u, v), new Color32(255, 255, 255, 0));
					verts[i4 + 3] = new Vertex(offset+bot_pos, V.XYZ(0, 0, -1), V.XY(u, v), new Color32(255, 255, 255, 0));

					if (i+1 < cornerResolution)
					{
						// Front slice
						inds[ind++] = i4 + 2;     inds[ind++] = i4_n + 2; inds[ind++] = cornerFront;
						// Back slice
						inds[ind++] = cornerBack; inds[ind++] = i4_n + 3; inds[ind++] = i4   + 3;
					}
					else
					{
						uint cornerFrontNext = (uint)(((c + 1) % 4) * (usubd + 2)) ;
						uint cornerBackNext  = cornerFrontNext + 1;
						i4_n = cornerBackNext + 1;

						// Front slice
						inds[ind++] = i4 + 2;         inds[ind++] = i4_n + 2; inds[ind++] = cornerFront;
						inds[ind++] = cornerFront;    inds[ind++] = i4_n + 2; inds[ind++] = cornerFrontNext;
						// Back slice
						inds[ind++] = cornerBack;     inds[ind++] = i4_n + 3; inds[ind++] = i4 + 3;
						inds[ind++] = cornerBackNext; inds[ind++] = i4_n + 3; inds[ind++] = cornerBack;
					}

					// Now edge strip
					inds[ind++] = i4_n + 1; inds[ind++] = i4_n;     inds[ind++] = i4;
					inds[ind++] = i4   + 1; inds[ind++] = i4_n + 1; inds[ind++] = i4;
				}
			}

			// Center quad front and back
			{
				uint tr_f = 0;
				uint tr_b = tr_f + 1;
				uint tl_f = usubd + 2;
				uint tl_b = tl_f + 1;
				uint bl_f = 2 * (usubd + 2);
				uint bl_b = bl_f + 1;
				uint br_f = 3 * (usubd + 2);
				uint br_b = br_f + 1;

				inds[ind++] = br_f; inds[ind++] = tr_f; inds[ind++] = tl_f;
				inds[ind++] = bl_f; inds[ind++] = br_f; inds[ind++] = tl_f;

				inds[ind++] = tl_b; inds[ind++] = tr_b; inds[ind++] = br_b;
				inds[ind++] = tl_b; inds[ind++] = br_b; inds[ind++] = bl_b;
			}

			mesh.SetVerts(verts);
			mesh.SetInds(inds);
			return mesh;
		}

		enum Side { Top, Bottom };
		static Mesh BackplateHalfMesh(float cornerRadius, int cornerResolution, Side side)
		{
			Mesh mesh = new Mesh();
	
			float radius = cornerRadius;

			int      subd       = cornerResolution * 4;
			uint     usubd      = (uint)subd;
			int      vert_count = 2 * (subd + 2) + 8;
			int      ind_count  = 2 * (cornerResolution * 12 + 6) + 12;
			Vertex[] verts      = new Vertex[vert_count];
			uint[]   inds       = new uint  [ind_count];

			uint  ind      = 0;
			Vec3  front    = V.XYZ(0, 0, 0.5f);
			float angStart = side == Side.Top ? 0 : 180;
			for (int c = 0; c < 2; c++)
			{
				float u = c == 0 || c == 3 ? 1 : -1;
				float v = c == 0 || c == 1 ? 1 : -1;
				Vec3 offset = V.XY0(-u*radius, -v*radius);

				uint vert = (uint)(c * (usubd + 2));
				verts[vert    ] = new Vertex(offset + front, V.XYZ(0, 0, 1), V.XY(u, v));
				verts[vert + 1] = new Vertex(offset - front, V.XYZ(0, 0,-1), V.XY(u, v));
				uint cornerFront = vert;
				uint cornerBack  = vert+1;

				for (uint i = 0; i < cornerResolution; i++)
				{
					float ang = (c*90 + (i/(float)(cornerResolution-1))*90 + angStart) * Units.deg2rad;
					float x = SKMath.Cos(ang);
					float y = SKMath.Sin(ang);
					Vec3 normal  = V.XY0(x,y);
					Vec3 top_pos = normal*radius + V.XYZ(0,0,0.5f);
					Vec3 bot_pos = normal*radius - V.XYZ(0,0,0.5f);

					uint i4   = vert+2 + i * 4;
					uint i4_n = vert+2 + (i + 1) * 4;

					// strip around the edge first
					verts[i4    ] = new Vertex(offset+top_pos, normal, V.XY(u, v), new Color32(255, 255, 255, 0));
					verts[i4 + 1] = new Vertex(offset+bot_pos, normal, V.XY(u, v), new Color32(255, 255, 255, 0));
					// now for the front and back faces
					verts[i4 + 2] = new Vertex(offset+top_pos, V.XYZ(0, 0,  1), V.XY(u, v), new Color32(255, 255, 255, 0));
					verts[i4 + 3] = new Vertex(offset+bot_pos, V.XYZ(0, 0, -1), V.XY(u, v), new Color32(255, 255, 255, 0));

					if (i+1 < 2)
					{
						// Front slice
						inds[ind++] = i4 + 2;     inds[ind++] = i4_n + 2; inds[ind++] = cornerFront;
						// Back slice
						inds[ind++] = cornerBack; inds[ind++] = i4_n + 3; inds[ind++] = i4   + 3;
					}

					// Now edge strip
					inds[ind++] = i4_n + 1; inds[ind++] = i4_n;     inds[ind++] = i4;
					inds[ind++] = i4   + 1; inds[ind++] = i4_n + 1; inds[ind++] = i4;
				}
			}

			// Center quad front and back
			{
				uint tr_f = 0;
				uint tr_b = tr_f + 1;
				uint tl_f = usubd + 2;
				uint tl_b = tl_f + 1;
				uint bl_f = 2 * (usubd + 2);
				uint bl_b = bl_f + 1;
				uint br_f = 3 * (usubd + 2);
				uint br_b = br_f + 1;

				inds[ind++] = br_f; inds[ind++] = tr_f; inds[ind++] = tl_f;
				inds[ind++] = bl_f; inds[ind++] = br_f; inds[ind++] = tl_f;

				inds[ind++] = tl_b; inds[ind++] = tr_b; inds[ind++] = br_b;
				inds[ind++] = tl_b; inds[ind++] = br_b; inds[ind++] = bl_b;
			}

			mesh.SetVerts(verts);
			mesh.SetInds(inds);
			return mesh;
		}

		static Mesh GlassButtonMesh(float cornerRadius, int cornerResolution)
		{
			Mesh mesh = new Mesh();
	
			float radius = cornerRadius;

			int      subd       = cornerResolution * 2;
			uint     usubd      = (uint)subd;
			int      vert_count = 4 * (subd + 2);
			int      ind_count  = 4 * (cornerResolution * 6 + 6) + 12;
			Vertex[] verts      = new Vertex[vert_count];
			uint[]   inds       = new uint  [ind_count];

			uint ind_f = (uint)(ind_count / 2);
			uint ind_b = 0;
			for (int c = 0; c < 4; c++)
			{
				float u = c == 0 || c == 3 ? 1 : -1;
				float v = c == 0 || c == 1 ? 1 : -1;
				Vec3 offset = V.XY0(-u*radius, -v*radius);
				Vec3 offsetFront = V.XY0(-u*radius, -v*radius);

				uint vert = (uint)(c * (usubd + 2));
				verts[vert    ] = new Vertex(offset      + V.XYZ(0,0,0.4f), V.XYZ(0, 0,-1), V.XY(u, v), new Color32(255, 0, 255, 255));
				verts[vert + 1] = new Vertex(offsetFront - V.XYZ(0,0,0.5f), V.XYZ(0, 0,-1), V.XY(u, v), new Color32(255, 255, 0, 255));
				uint cornerBack  = vert;
				uint cornerFront = vert+1;

				for (uint i = 0; i < cornerResolution; i++)
				{
					float ang = (c*90 + (i/(float)(cornerResolution-1))*90) * Units.deg2rad;
					float x = SKMath.Cos(ang);
					float y = SKMath.Sin(ang);
					Vec3 normal   = V.XY0(x,y);
					Vec3 backPos  = normal*radius + V.XYZ(0,0,0.4f);
					Vec3 FrontPos = normal*radius*0.75f - V.XYZ(0,0,0.5f);

					uint i4   = vert+2 + i * 2;
					uint i4_n = vert+2 + (i + 1) * 2;

					// Front and back faces
					verts[i4    ] = new Vertex(offset     +backPos,  V.XYZ(0, 0, -1), V.XY(u, v), new Color32(255, 0, 255, 255));
					verts[i4 + 1] = new Vertex(offsetFront+FrontPos, V.XYZ(0, 0, -1), V.XY(u, v), new Color32(255, 255, 0, 0));

					if (i+1 < cornerResolution)
					{
						// Back slice
						inds[ind_b++] = cornerBack;  inds[ind_b++] = i4_n;     inds[ind_b++] = i4;
						// Front slice
						inds[ind_f++] = cornerFront; inds[ind_f++] = i4_n + 1; inds[ind_f++] = i4 + 1;
					}
					else
					{
						uint cornerBackNext = (uint)(((c + 1) % 4) * (usubd + 2)) ;
						uint cornerFrontNext  = cornerBackNext + 1;
						i4_n = cornerFrontNext + 1;

						// Back slice
						inds[ind_b++] = cornerBack;     inds[ind_b++] = i4_n; inds[ind_b++] = i4;
						inds[ind_b++] = cornerBackNext; inds[ind_b++] = i4_n; inds[ind_b++] = cornerBack;
						// Front slice
						inds[ind_f++] = cornerFront;     inds[ind_f++] = i4_n + 1; inds[ind_f++] = i4 + 1;
						inds[ind_f++] = cornerFrontNext; inds[ind_f++] = i4_n + 1; inds[ind_f++] = cornerFront;
					}
				}
			}

			// Center quad front and back
			{
				uint tr_f = 0;
				uint tr_b = tr_f + 1;
				uint tl_f = usubd + 2;
				uint tl_b = tl_f + 1;
				uint bl_f = 2 * (usubd + 2);
				uint bl_b = bl_f + 1;
				uint br_f = 3 * (usubd + 2);
				uint br_b = br_f + 1;

				inds[ind_b++] = tl_f; inds[ind_b++] = tr_f; inds[ind_b++] = br_f;
				inds[ind_b++] = tl_f; inds[ind_b++] = br_f; inds[ind_b++] = bl_f;

				inds[ind_f++] = tl_b; inds[ind_f++] = tr_b; inds[ind_f++] = br_b;
				inds[ind_f++] = tl_b; inds[ind_f++] = br_b; inds[ind_f++] = bl_b;
			}

			mesh.SetVerts(verts);
			mesh.SetInds(inds);
			return mesh;
		}
	}
}
