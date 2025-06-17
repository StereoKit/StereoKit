// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

namespace StereoKit
{
	public class Interactor
	{
		private int _inst;

		private Interactor(int inst)
		{
			_inst = inst;
		}

		public float MinDistance {
			get => NativeAPI.interactor_get_min_distance(_inst);
			set => NativeAPI.interactor_set_min_distance(_inst, value);
		}
		public float Radius {
			get => NativeAPI.interactor_get_radius(_inst);
			set => NativeAPI.interactor_set_radius(_inst, value);
		}
		public BtnState Tracked => NativeAPI.interactor_get_tracked(_inst);
		public IdHash   Focused => NativeAPI.interactor_get_focused(_inst);
		public IdHash   Active  => NativeAPI.interactor_get_active (_inst);
		public Pose     Motion  => NativeAPI.interactor_get_motion (_inst);

		public void Update(Vec3 capsuleStart, Vec3 capsuleEnd, Pose motion, Vec3 motionAnchor, Vec3 secondaryMotion, BtnState active, BtnState tracked)
			=> NativeAPI.interactor_update(_inst, capsuleStart, capsuleEnd, motion, motionAnchor, secondaryMotion, active, tracked);

		public void Destroy()
			=> NativeAPI.interactor_destroy(_inst);

		public bool TryGetFocusBounds(out Pose poseWorld, out Bounds boundsLocal)
			=> NativeAPI.interactor_get_focus_bounds(_inst, out poseWorld, out boundsLocal);

		public static Interactor Create(InteractorType shapeType, InteractorEvent events, InteractorActivation activationType, int inputSourceId, float capsuleRadius, int secondaryMotionDimensions)
			=> new Interactor(NativeAPI.interactor_create(shapeType, events, activationType, inputSourceId, capsuleRadius, secondaryMotionDimensions));
	}
}
