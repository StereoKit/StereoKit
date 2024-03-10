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
				if      (info.MemberType == typeof(int    )) { value = info.GetValue<int  >(inst); if (step == 0) step = 1; }
				else if (info.MemberType == typeof(long   )) { value = info.GetValue<long >(inst); if (step == 0) step = 1; }
				else if (info.MemberType == typeof(float  )) value = info.GetValue<float>(inst);
				else if (info.MemberType == typeof(decimal)) value = (float)info.GetValue<decimal>(inst);
				else { UI.Label($"{info.info.Name}: doesn't support slider", false); return; }

				UI.Label($"{info.info.Name}:", new Vec2(Inspector.LabelWidth,0), false);
				UI.SameLine();
				UI.Label($"{value:0.####}", false);
				if (UI.HSlider(info.info.Name, ref value, min, max, step)) {
					if      (info.MemberType == typeof(int    )) info.SetValue<int    >(inst, (int    )value);
					else if (info.MemberType == typeof(long   )) info.SetValue<long   >(inst, (long   )value);
					else if (info.MemberType == typeof(float  )) info.SetValue<float  >(inst,          value);
					else if (info.MemberType == typeof(decimal)) info.SetValue<decimal>(inst, (decimal)value);
				}
			}
		}

		///////////////////////////////////////////

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
			public static int Match(Type type) => type == typeof(Pose) ? 1 : 0;
		}

		///////////////////////////////////////////
		
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
			public static int Match(Type type) => type == typeof(Model) ? 10 : 0;
		}

		///////////////////////////////////////////

		internal class MaterialDefaultAttribute : Attribute, InspectorTypeAttribute
		{
			public void Draw(InspectorMember info, object inst)
			{
				UI.Label($"{info.info.Name}:", new Vec2(LabelWidth, 0), false);
				UI.SameLine();
				Bounds b = UI.LayoutReserve(UI.LayoutRemaining.x, UI.LineHeight, false, UI.LineHeight);
				Mesh.Sphere.Draw(info.GetValue<Material>(inst), Matrix.TS(b.center, b.dimensions.y));
				
			}
			public static int Match(Type type) => type == typeof(Material) ? 10 : 0;
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

		internal class ToggleDefaultAttribute : Attribute, InspectorTypeAttribute
		{
			public void Draw(InspectorMember info, object inst)
			{
				bool value = info.GetValue<bool>(inst);
				if (UI.Toggle(info.info.Name, ref value))
					info.SetValue(inst, value);
			}
			public static int Match(Type type) => type == typeof(bool) ? 10 : 0;
		}

		///////////////////////////////////////////

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
			public static int Match(Type type) => type == typeof(string) ? 10 : 0;
		}

		///////////////////////////////////////////

		internal class AnyDefaultAttribute : Attribute, InspectorTypeAttribute
		{
			public void Draw(InspectorMember info, object inst)
			{
				UI.Label($"{info.info.Name}:", new Vec2(Inspector.LabelWidth, 0), false);
				UI.SameLine();
				UI.Label($"{info.GetValue<object>(inst)}");
			}
			public static int Match(Type type) => 1;
		}
	}
}
