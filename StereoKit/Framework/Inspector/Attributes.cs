using System;

namespace StereoKit.Framework
{
	partial class Inspector
	{
		///////////////////////////////////////////
		
		public class SliderAttribute : Attribute, InspectorTypeAttribute
		{
			private float min;
			private float max;
			private float step;

			public SliderAttribute(float min, float max, float step = 0)
			{
				this.min  = min;
				this.max  = max;
				this.step = step;
			}

			public void Draw(InspectorMember info, object inst)
			{
				float value;
				if      (info.MemberType == typeof(int   )) { value = info.GetValue<int  >(inst); if (step == 0) step = 1; }
				else if (info.MemberType == typeof(long  )) { value = info.GetValue<long >(inst); if (step == 0) step = 1; }
				else if (info.MemberType == typeof(float )) value =        info.GetValue<float >(inst);
				else if (info.MemberType == typeof(double)) value = (float)info.GetValue<double>(inst);
				else { UI.Label($"{info.info.Name}: doesn't support slider", false); return; }

				UI.Label($"{info.info.Name}:", new Vec2(Inspector.LabelWidth,0), false);
				UI.SameLine();
				UI.Label($"{value:0.###}", new Vec2(0.03f,0), false);
				UI.SameLine();
				if (UI.HSlider(info.info.Name, ref value, min, max, step)) {
					if      (info.MemberType == typeof(int   )) info.SetValue<int   >(inst, (int   )value);
					else if (info.MemberType == typeof(long  )) info.SetValue<long  >(inst, (long  )value);
					else if (info.MemberType == typeof(float )) info.SetValue<float >(inst,         value);
					else if (info.MemberType == typeof(double)) info.SetValue<double>(inst, (double)value);
				}
			}
		}

		///////////////////////////////////////////

		[MatchType(typeof(Pose), priority = 2)]
		public class PoseAttribute : Attribute, InspectorTypeAttribute
		{
			bool   enabled;
			Bounds bounds;
			Func<object, Bounds> getBounds;

			public PoseAttribute(float boundsSize) => bounds = new Bounds(Vec3.One*boundsSize);
			public PoseAttribute(Vec3 boundsSize) => bounds = new Bounds(boundsSize);
			public PoseAttribute(Bounds bounds) => this.bounds = bounds;
			public PoseAttribute(Func<object, Bounds> getBounds) => this.getBounds = getBounds;
			public PoseAttribute() : this(0.04f) { }

			public void Draw(InspectorMember info, object inst)
			{
				UI.Label(info.info.Name, new Vec2(Inspector.LabelWidth,0), false);
				UI.SameLine();
				UI.Toggle("Edit Pose", ref enabled);
				
				if (enabled)
				{
					if (getBounds != null)
						bounds = getBounds(inst);

					Pose value = Hierarchy.ToLocal(info.GetValue<Pose>(inst));
					if (UI.Handle(info.info.Name, ref value, bounds))
						info.SetValue(inst, Hierarchy.ToWorld(value));
					Lines.AddAxis(value, bounds.dimensions.y/2);
					Mesh.Cube.Draw(Material.UIBox, Matrix.TS(bounds.center, bounds.dimensions)*value.ToMatrix());
				}
			}
		}

		///////////////////////////////////////////

		[MatchType(typeof(Model), priority = 2)]
		internal class ModelDefaultAttribute : Attribute, InspectorTypeAttribute
		{
			public void Draw(InspectorMember info, object inst)
			{
				UI.Label($"{info.info.Name}:", new Vec2(LabelWidth, 0), false);
				UI.SameLine();
				if (UI.Button("Load"))
					Platform.FilePicker(PickerMode.Open, (file) => info.SetValue(inst, Model.FromFile(file)), null, Assets.ModelFormats);
				UI.SameLine();
				UI.Model(info.GetValue<Model>(inst), new Vec2(0, UI.LineHeight-UI.Settings.padding*2));
				
			}
		}

		///////////////////////////////////////////

		[MatchType(typeof(Material), priority = 2)]
		internal class MaterialDefaultAttribute : Attribute, InspectorTypeAttribute
		{
			public void Draw(InspectorMember info, object inst)
			{
				UI.Label($"{info.info.Name}:", new Vec2(LabelWidth, 0), false);
				UI.SameLine();
				Bounds b = UI.LayoutReserve(UI.LayoutRemaining.x, UI.LineHeight, false, UI.LineHeight);
				Mesh.Sphere.Draw(info.GetValue<Material>(inst), Matrix.TS(b.center, b.dimensions.y));
				
			}
		}

		///////////////////////////////////////////

		public class SeparatorAttribute : Attribute, InspectorExtraAttribute
		{
			public void Draw(object inst) => UI.HSeparator();
		}

		///////////////////////////////////////////

		public class HideAttribute : Attribute { }

		///////////////////////////////////////////

		public class ShowAttribute : Attribute { }

		///////////////////////////////////////////

		[MatchType(typeof(bool), priority = 2)]
		internal class ToggleDefaultAttribute : Attribute, InspectorTypeAttribute
		{
			public void Draw(InspectorMember info, object inst)
			{
				bool value = info.GetValue<bool>(inst);
				UI.Label($"{info.info.Name}:", new Vec2(LabelWidth, 0), false);
				UI.SameLine();
				if (UI.Toggle(info.info.Name, ref value))
					info.SetValue(inst, value);
			}
		}

		///////////////////////////////////////////

		[MatchType(typeof(string), priority = 2)]
		internal class InputDefaultAttribute : Attribute, InspectorTypeAttribute
		{
			public void Draw(InspectorMember info, object inst)
			{
				UI.Label($"{info.info.Name}:", new Vec2(LabelWidth, 0), false);
				UI.SameLine();

				string value = info.GetValue<string>(inst);
				if (UI.Input(info.info.Name, ref value, new Vec2(UI.LayoutRemaining.x > LabelWidth * 2 ? 0 : LabelWidth * 2, 0)))
					info.SetValue(inst, value);
			}
		}

		///////////////////////////////////////////

		[MatchType(types = null, priority = 1)]
		internal class AnyDefaultAttribute : Attribute, InspectorTypeAttribute
		{
			public void Draw(InspectorMember info, object inst)
			{
				UI.Label($"{info.info.Name}:", new Vec2(Inspector.LabelWidth, 0), false);
				UI.SameLine();
				if (info.MemberType.IsPrimitive) {
					UI.Label($"{info.GetValue<object>(inst)}");
				} else {
					UI.Label($"{info.GetValue<object>(inst)}", new Vec2(UI.LayoutRemaining.x-(UI.LineHeight+UI.Settings.gutter),0));
					UI.SameLine();
					if (UI.ButtonImg("Visit", Sprite.ArrowRight, UIBtnLayout.CenterNoText))
						Inspector.Push(info.GetValue<object>(inst));
				}
			}
		}

		///////////////////////////////////////////
		
		public class MatchTypeAttribute : Attribute
		{
			public Type[] types;
			public int    priority = 10;

			public MatchTypeAttribute(params Type[] types)
			{
				this.types = types;
			}

			public int Match(Type type)
			{
				if (types == null) return 1;
				return Array.IndexOf(types, type) >= 0 ? priority : 0;
			}
		}
	}
}
