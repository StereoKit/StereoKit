// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

namespace StereoKit.Framework
{
	public class MaterialInspector : IStepper
	{
		static MaterialInspector _inst;

		Material   material;
		Material[] texPreviews;
		bool       visible;

		public Pose pose;

		public bool Enabled => true;

		public static void Show(Material material)
		{
			if (_inst == null)
				_inst = SK.AddStepper<MaterialInspector>();
			if (!_inst.visible) _inst.pose = UI.PopupPose();
			_inst.material = material;
			_inst.visible  = true;
			_inst.BuildTexPreviews();
		}

		public bool Initialize() => true;

		void BuildTexPreviews()
		{
			int texCount = 0;
			foreach (MatParamInfo p in material.GetAllParamInfo())
				if (p.type == MaterialParam.Texture) texCount++;

			texPreviews = new Material[texCount];
			int idx = 0;
			foreach (MatParamInfo p in material.GetAllParamInfo())
			{
				if (p.type != MaterialParam.Texture) continue;
				texPreviews[idx] = Material.Unlit.Copy();
				Tex tex = material.GetTexture(p.name);
				if (tex != null)
					texPreviews[idx].SetTexture("diffuse", tex);
				idx++;
			}
		}
		public void Shutdown() { if (_inst == this) _inst = null; }

		public void Step()
		{
			if (!visible || material == null) return;

			UI.WindowBegin("Material Inspector", ref pose, new Vec2(60 * U.cm, 0));
			UI.LayoutPushCut(UICut.Left, 0.26f);
				DrawHeader();
				UI.HSeparator();
				DrawRenderState();
				if (material.Chain != null)
				{
					UI.HSeparator();
					DrawChain();
				}
			UI.LayoutPop();

			DrawParams();
			UI.WindowEnd();
		}

		void DrawHeader()
		{
			// Material preview sphere
			Bounds b = UI.LayoutReserve(V.XY(UI.LineHeight * 3, UI.LineHeight * 3));
			Mesh.Sphere.Draw(material, Matrix.TS(b.center, b.dimensions.x * 0.8f));

			UI.SameLine();
			UI.Text($"{material.Id ?? "(none)"}", Align.CenterLeft, TextFit.Clip | TextFit.Wrap, new Vec2(UI.LayoutRemaining.x, UI.LineHeight*3));
		}

		void DrawRenderState()
		{
			UI.PanelBegin();
			UI.Label("Render State", new Vec2(UI.LayoutRemaining.x,0));
			UI.PanelEnd();

			float width = UI.LayoutRemaining.x - UI.Settings.gutter;
			Vec2 size1 = new Vec2(width *0.65f, UI.LineHeight);
			Vec2 size2 = new Vec2(width *0.35f, UI.LineHeight);


			UI.Label("Transparency:",                  size1); UI.SameLine();
			UI.Label(material.Transparency.ToString(), size2);

			UI.Label("Face Cull:",                     size1); UI.SameLine();
			UI.Label(material.FaceCull.ToString(),     size2);

			UI.Label("Depth Test:",                    size1); UI.SameLine();
			UI.Label(material.DepthTest.ToString(),    size2);

			UI.Label("Queue Offset:",                  size1); UI.SameLine();
			UI.Label(material.QueueOffset.ToString(),  size2);

			bool depthWrite = material.DepthWrite;
			if (UI.Toggle("Depth Write", ref depthWrite))
				material.DepthWrite = depthWrite;

			bool depthClamp = material.DepthClamp;
			if (UI.Toggle("Depth Clamp", ref depthClamp))
				material.DepthClamp = depthClamp;

			bool wireframe = material.Wireframe;
			if (UI.Toggle("Wireframe", ref wireframe))
				material.Wireframe = wireframe;
		}

		void DrawParams()
		{
			Vec2 labelSize = new Vec2(UI.LineHeight * 3, UI.LineHeight);

			UI.PanelBegin();
			UI.Label($"Shader ({material.Shader?.Name ?? "(none)"})", new Vec2(UI.LayoutRemaining.x,0));
			UI.PanelEnd();
			int texIdx = 0;
			foreach (MatParamInfo p in material.GetAllParamInfo())
			{
				if (p.name == "sk_inst" || p.name == "sk_cubemap") continue;

				switch (p.type)
				{
				case MaterialParam.Float:
					UI.Label(p.name, labelSize);
					UI.SameLine();
					UI.Label($"{material.GetFloat(p.name):F4}", false);
					break;
				case MaterialParam.Color128:
					DrawColorParam(p.name);
					break;
				case MaterialParam.Vector2:
				{
					UI.Label(p.name, labelSize);
					UI.SameLine();
					Vec2 v = material.GetVector2(p.name);
					UI.Label($"({v.x:F3}, {v.y:F3})", false);
					break;
				}
				case MaterialParam.Vector3:
				{
					UI.Label(p.name, labelSize);
					UI.SameLine();
					Vec3 v = material.GetVector3(p.name);
					UI.Label($"({v.x:F3}, {v.y:F3}, {v.z:F3})", false);
					break;
				}
				case MaterialParam.Vector4:
				{
					UI.Label(p.name, labelSize);
					UI.SameLine();
					Vec4 v = material.GetVector4(p.name);
					UI.Label($"({v.x:F3}, {v.y:F3}, {v.z:F3}, {v.w:F3})", false);
					break;
				}
				case MaterialParam.Int:
					UI.Label(p.name, labelSize);
					UI.SameLine();
					UI.Label($"{material.GetInt(p.name)}", false);
					break;
				case MaterialParam.UInt:
					UI.Label(p.name, labelSize);
					UI.SameLine();
					UI.Label($"{material.GetUInt(p.name)}", false);
					break;
				case MaterialParam.Matrix:
					UI.Label(p.name, labelSize);
					UI.SameLine();
					UI.Label("(matrix)", false);
					break;
				case MaterialParam.Texture:
					DrawTextureParam(p.name, texIdx);
					texIdx++;
					break;
				default:
					UI.Label(p.name, labelSize);
					UI.SameLine();
					UI.Label($"({p.type})", false);
					break;
				}
			}
		}

		void DrawColorParam(string name)
		{
			Color c = material.GetColor(name);
			Bounds b = UI.LayoutReserve(V.XY(UI.LineHeight, UI.LineHeight));
			b.center.z -= b.dimensions.x * 0.5f;
			Mesh.Cube.Draw(Material.Unlit, Matrix.TS(b.center, b.dimensions.x * 0.7f), c);

			UI.SameLine();
			UI.Label($"{name}: ({c.r:F2}, {c.g:F2}, {c.b:F2}, {c.a:F2})", false);
		}

		void DrawTextureParam(string name, int texIdx)
		{
			Tex tex = material.GetTexture(name);

			float thumbSize = UI.LineHeight * 3;
			Bounds b = UI.LayoutReserve(V.XY(thumbSize, thumbSize));
			b.center.z -= 0.005f;
			if (tex != null && texIdx < texPreviews.Length)
				Mesh.Quad.Draw(texPreviews[texIdx], Matrix.TS(b.center, thumbSize ));

			UI.SameLine();
			if (tex != null)
				UI.Text($"{name}\n{tex.Width}x{tex.Height} {tex.Format}\n{tex?.Id ?? "(no id)"}");
		}

		void DrawChain()
		{
			Material chain = material.Chain;
			if (chain != null)
			{
				if (UI.Button($"Chain: {chain.Id ?? "(no id)"}"))
					Show(chain);
			}

			for (int i = 1; i < 4; i++)
			{
				Material variant = material.GetVariant(i);
				if (variant != null)
				{
					if (UI.Button($"Variant {i}: {variant.Id ?? "(no id)"}"))
						Show(variant);
				}
			}
		}
	}
}
