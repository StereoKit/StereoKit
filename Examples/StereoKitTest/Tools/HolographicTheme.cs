// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using System.Collections.Generic;

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
			uiGlassMaterial.DepthWrite   = false;
			uiGlassMaterial.DepthTest    = DepthTest.LessOrEq;
			uiGlassMaterial.QueueOffset  = -20;
			uiGlassMaterial[MatParamName.ColorTint] = Color.White;

			Mesh quadrantCube = Mesh.GenerateCube(Vec3.One);
			UI.QuadrantSizeMesh(ref quadrantCube);

			Mesh backplate      = BackplateMesh(.01f, 6);
			Mesh backplateSmall = BackplateMesh(.004f, 6);
			Mesh glassMesh      = GlassButtonMesh(.01f, 6, 0.75f);
			Mesh roundMesh      = GlassButtonMesh(UI.LineHeight/2, 6, 1.0f);
			Mesh panelMesh      = GlassPanel(.01f, 6);
			UI.SetElementVisual(UIVisual.Default,     backplate,      uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.WindowHead,  backplate,      uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.WindowBody,  backplate,      uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.Separator,   quadrantCube,   uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.SliderLine,  backplateSmall, uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.SliderLineActive,   backplateSmall, uiQuadrantMaterial, V.XY(0.008f, 0.008f));
			UI.SetElementVisual(UIVisual.SliderLineInactive, backplateSmall, uiQuadrantMaterial, V.XY(0.008f, 0.008f));
			UI.SetElementVisual(UIVisual.SliderPinch, backplateSmall, uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.SliderPush,  backplateSmall, uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.Button,      glassMesh,      uiGlassMaterial);
			UI.SetElementVisual(UIVisual.ButtonRound, roundMesh,      uiGlassMaterial);
			UI.SetElementVisual(UIVisual.Toggle,      glassMesh,      uiGlassMaterial);
			UI.SetElementVisual(UIVisual.Panel,       panelMesh,      uiQuadrantMaterial);

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

		public static void ApplyMeshesOnly()
		{
			Shader   uiQuadrantShader   = Shader.FromFile("edged_quadrant.hlsl");
			Material uiQuadrantMaterial = new Material(uiQuadrantShader);

			Mesh backplate      = BackplateMesh    (.01f,  6);
			Mesh backplateTop   = BackplateHalfMesh(.01f,  6, Side.Top);
			Mesh backplateBot   = BackplateHalfMesh(.01f,  6, Side.Bottom);
			Mesh backplateSmall = BackplateMesh    (.004f, 6);
			UI.SetElementVisual(UIVisual.Default,        backplate,      uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.WindowHead,     backplateTop,   uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.WindowBody,     backplateBot,   uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.WindowBodyOnly, backplate,      uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.WindowHeadOnly, backplate,      uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.SliderLine,     backplateSmall, uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.SliderPinch,    backplateSmall, uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.SliderPush,     backplateSmall, uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.Button,         backplate,      uiQuadrantMaterial);
			UI.SetElementVisual(UIVisual.Toggle,         backplate,      uiQuadrantMaterial);
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
			int      vert_count = 2 * (subd + 2) + 12;
			int      ind_count  = 2 * (cornerResolution * 12 + 6) + 48;
			Vertex[] verts      = new Vertex[vert_count];
			uint[]   inds       = new uint  [ind_count];

			uint  ind      = 0;
			Vec3  front    = V.XYZ(0, 0, 0.5f);
			float angStart = side == Side.Top ? 0 : 180;
			for (int c = 0; c < 2; c++)
			{
				int   cr = side == Side.Top ? c: c+2;
				float u  = cr == 0 || cr == 3 ? 1 : -1;
				float v  = cr == 0 || cr == 1 ? 1 : -1;
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

					if (i + 1 < cornerResolution)
					{
						// Front slice
						inds[ind++] = i4 + 2;     inds[ind++] = i4_n + 2; inds[ind++] = cornerFront;
						// Back slice
						inds[ind++] = cornerBack; inds[ind++] = i4_n + 3; inds[ind++] = i4   + 3;
					}
					else if (c + 1 < 2)
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

					if (i + 1 < cornerResolution || c+1 < 2)
					{
						// Now edge strip
						inds[ind++] = i4_n + 1; inds[ind++] = i4_n;     inds[ind++] = i4;
						inds[ind++] = i4   + 1; inds[ind++] = i4_n + 1; inds[ind++] = i4;
					}
				}
			}

			// bottom half
			{
				uint start = (uint)(2 * (usubd + 2));
				int   cr = side == Side.Top ? 2 : 0;
				float u  = cr == 0 || cr == 3 ? 1 : -1;
				float v  = cr == 0 || cr == 1 ? 1 : -1;
				Vec3 offset = V.XY0(-u * radius, 0);
				Vec3 normal = V.XY0(SKMath.Cos((angStart + 180) * Units.deg2rad), SKMath.Sin((angStart + 180) * Units.deg2rad) * 2);
				verts[start    ] = new Vertex(offset + front, V.XYZ(0, 0, 1), V.XY(u, v));
				verts[start + 1] = new Vertex(offset - front, V.XYZ(0, 0,-1), V.XY(u, v));
				verts[start + 2] = new Vertex(offset + normal*radius + front, V.XYZ(0, 0, 1), V.XY(u, v), new Color32(255, 255, 255, 0));
				verts[start + 3] = new Vertex(offset + normal*radius - front, V.XYZ(0, 0,-1), V.XY(u, v), new Color32(255, 255, 255, 0));
				verts[start + 4] = new Vertex(offset + normal*radius + front, new Vec3(normal.x,0,0), V.XY(u, v), new Color32(255, 255, 255, 0));
				verts[start + 5] = new Vertex(offset + normal*radius - front, new Vec3(normal.x,0,0), V.XY(u, v), new Color32(255, 255, 255, 0));

				cr = side == Side.Top ? 3 : 1;
				u  = cr == 0 || cr == 3 ? 1 : -1;
				v  = cr == 0 || cr == 1 ? 1 : -1;
				offset = V.XY0(-u * radius, 0);
				normal = V.XY0(SKMath.Cos(angStart * Units.deg2rad), SKMath.Sin(angStart * Units.deg2rad)*2);
				verts[start + 6] = new Vertex(offset + front, V.XYZ(0, 0, 1), V.XY(u, v));
				verts[start + 7] = new Vertex(offset - front, V.XYZ(0, 0,-1), V.XY(u, v));
				verts[start + 8] = new Vertex(offset + normal*radius + front, V.XYZ(0, 0, 1), V.XY(u, v), new Color32(255, 255, 255, 0));
				verts[start + 9] = new Vertex(offset + normal*radius - front, V.XYZ(0, 0,-1), V.XY(u, v), new Color32(255, 255, 255, 0));
				verts[start + 10] = new Vertex(offset + normal*radius + front, new Vec3(normal.x,0,0), V.XY(u, v), new Color32(255, 255, 255, 0));
				verts[start + 11] = new Vertex(offset + normal*radius - front, new Vec3(normal.x,0,0), V.XY(u, v), new Color32(255, 255, 255, 0));

				uint taf = 2;
				uint tab = 3;
				uint tbf = 4;
				uint tbb = 5;
				uint tcf = 0;
				uint tcb = 1;
				uint tdf = usubd + 2;
				uint tdb = usubd + 3;
				uint tef = start - 2;
				uint teb = start - 1;
				uint tff = start - 4;
				uint tfb = start - 3;

				uint baf = start + 10;
				uint bab = start + 11;
				uint bbf = start + 8;
				uint bbb = start + 9;
				uint bcf = start + 6;
				uint bcb = start + 7;
				uint bdf = start + 0;
				uint bdb = start + 1;
				uint bef = start + 2;
				uint beb = start + 3;
				uint bff = start + 4;
				uint bfb = start + 5;

				inds[ind++] = tbf; inds[ind++] = tcf; inds[ind++] = bbf;
				inds[ind++] = tcf; inds[ind++] = bcf; inds[ind++] = bbf;

				inds[ind++] = tcf; inds[ind++] = tdf; inds[ind++] = bcf;
				inds[ind++] = tdf; inds[ind++] = bdf; inds[ind++] = bcf;

				inds[ind++] = tdf; inds[ind++] = tef; inds[ind++] = bdf;
				inds[ind++] = tef; inds[ind++] = bef; inds[ind++] = bdf;

				inds[ind++] = bbb; inds[ind++] = tcb; inds[ind++] = tbb;
				inds[ind++] = bbb; inds[ind++] = bcb; inds[ind++] = tcb;

				inds[ind++] = bcb; inds[ind++] = tdb; inds[ind++] = tcb;
				inds[ind++] = bcb; inds[ind++] = bdb; inds[ind++] = tdb;

				inds[ind++] = bdb; inds[ind++] = teb; inds[ind++] = tdb;
				inds[ind++] = bdb; inds[ind++] = beb; inds[ind++] = teb;

				inds[ind++] = bab; inds[ind++] = tab; inds[ind++] = taf;
				inds[ind++] = baf; inds[ind++] = bab; inds[ind++] = taf;

				inds[ind++] = tff; inds[ind++] = tfb; inds[ind++] = bfb;
				inds[ind++] = tff; inds[ind++] = bfb; inds[ind++] = bff;
			}

			mesh.SetVerts(verts);
			mesh.SetInds(inds);
			return mesh;
		}

		struct MeshData
		{
			public List<Vertex> verts;
			public List<uint>   inds;

			public MeshData()
			{
				verts = new List<Vertex>();
				inds  = new List<uint>();
			}
			public Mesh MakeMesh()
			{
				Mesh mesh = new Mesh();
				mesh.SetData(verts.ToArray(), inds.ToArray());
				return mesh;
			}
		}

		static Mesh GlassButtonMesh(float cornerRadius, uint cornerResolution, float topRadiusScale)
		{
			MeshData data = new MeshData();
		
			AddRoundedPlane(ref data,  0.4f, cornerRadius, 1.0f,           cornerResolution, new Color32(255, 0, 255, 255), new Color32(255, 0, 255, 255));
			AddRoundedPlane(ref data, -0.5f, cornerRadius, topRadiusScale, cornerResolution, new Color32(255, 255, 0, 255), new Color32(255, 255, 0, 0));
			return data.MakeMesh();
		}

		static Mesh GlassPanel(float cornerRadius, uint cornerResolution)
		{
			MeshData data = new MeshData();
			AddRoundedPlane(ref data, 0, cornerRadius, 1, cornerResolution, new Color32(255, 0, 255, 255), new Color32(255, 0, 255, 255));
			return data.MakeMesh();
		}

		static void AddRoundedPlane(ref MeshData data, float zOffset, float cornerRadius, float cornerScalePercent, uint cornerResolution, Color32 innerColor, Color32 outerColor)
		{
			uint     vert_count = 4 * cornerResolution + 4;
			uint     ind_count  = 4 * (cornerResolution * 3 + 3) + 6;
			Vertex[] verts      = new Vertex[vert_count];
			uint[]   inds       = new uint  [ind_count];
			uint     indStart   = (uint)data.verts.Count;

			uint ind_b = 0;
			for (uint c = 0; c < 4; c++)
			{
				float u = c == 0 || c == 3 ? 1 : -1;
				float v = c == 0 || c == 1 ? 1 : -1;
				Vec3 offset  = V.XY0(-u*cornerRadius, -v*cornerRadius);

				// inner corner
				uint innerCurr = c * (cornerResolution + 1);
				verts[innerCurr] = new Vertex(offset + V.XYZ(0,0,zOffset), V.XYZ(0, 0,-1), V.XY(u, v), innerColor);

				for (uint i = 0; i < cornerResolution; i++)
				{
					float ang = (c*90 + (i/(float)(cornerResolution-1))*90) * Units.deg2rad;
					float x   = SKMath.Cos(ang);
					float y   = SKMath.Sin(ang);
					Vec3 normal  = V.XY0(x,y);
					Vec3 edgePos = normal*cornerRadius*cornerScalePercent;
					edgePos.z = zOffset;

					// rounded corner
					uint outerCurr = innerCurr + 1 + i;
					verts[outerCurr] = new Vertex(offset + edgePos, V.XYZ(0, 0, -1), V.XY(u, v), outerColor);

					if (i+1 < cornerResolution)
					{
						// Connect the triangle fan on the corner itself
						uint outerNext = innerCurr + 1 + i + 1;
						inds[ind_b++] = indStart + innerCurr; inds[ind_b++] = indStart + outerNext; inds[ind_b++] = indStart + outerCurr;
					}
					else
					{
						// Connect a quad to the next corner
						uint innerNext = ((c+1)%4) * (cornerResolution+1);
						uint outerNext = innerNext + 1;

						inds[ind_b++] = indStart + innerCurr; inds[ind_b++] = indStart + outerNext; inds[ind_b++] = indStart + outerCurr;
						inds[ind_b++] = indStart + innerCurr; inds[ind_b++] = indStart + innerNext; inds[ind_b++] = indStart + outerNext;
					}
				}
			}

			// Center quad
			{
				uint tr_f = indStart + 0;
				uint tl_f = indStart + cornerResolution + 1;
				uint bl_f = indStart + 2 * (cornerResolution + 1);
				uint br_f = indStart + 3 * (cornerResolution + 1);

				inds[ind_b++] = tl_f; inds[ind_b++] = tr_f; inds[ind_b++] = br_f;
				inds[ind_b++] = tl_f; inds[ind_b++] = br_f; inds[ind_b++] = bl_f;
			}

			data.verts.AddRange(verts);
			data.inds .AddRange(inds );
		}
	}
}
