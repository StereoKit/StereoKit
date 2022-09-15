#include "../stereokit.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../systems/input.h"
#include "input_hand.h"
#include "hand_poses.h"

#include "hand_mouse.h"
#include "hand_override.h"
#include "hand_oxr_controller.h"
#include "hand_oxr_articulated.h"

#include "../platforms/platform_utils.h"

#include <math.h>
#include <string.h>

namespace sk {

///////////////////////////////////////////

#define SK_FINGERS 5
#define SK_FINGERJOINTS 5
#define SK_SQRT2 1.41421356237f
#define SK_FINGER_SOLIDS 1

struct hand_state_t {
	hand_t      info;
	pose_t      pose_blend[5][5];
	solid_t     solids[SK_FINGER_SOLIDS];
	material_t  material;
	hand_mesh_t mesh;
	bool        visible;
	bool        enabled;
	bool        solid;
};

typedef struct hand_system_t {
	hand_system_ system;
	float pinch_blend;
	bool (*available)();
	void (*init)();
	void (*shutdown)();
	void (*update_inactive)();
	void (*update_frame)();
	void (*update_poses)(bool update_visuals);
} hand_system_t;

hand_system_t hand_sources[] = { // In order of priority
	{ hand_system_override,
		0.2f,
		hand_override_available,
		hand_override_init,
		hand_override_shutdown,
		nullptr,
		hand_override_update_frame,
		hand_override_update_poses },
#if defined(SK_XR_OPENXR)
	{ hand_system_oxr_articulated,
		0.2f,
		hand_oxra_available,
		hand_oxra_init,
		hand_oxra_shutdown,
		hand_oxra_update_inactive,
		hand_oxra_update_frame,
		hand_oxra_update_poses },
	{ hand_system_oxr_controllers,
		0.6f,
		hand_oxrc_available,
		hand_oxrc_init,
		hand_oxrc_shutdown,
		nullptr,
		hand_oxrc_update_frame,
		hand_oxrc_update_poses },
#endif
	{ hand_system_mouse,
		1,
		hand_mouse_available,
		hand_mouse_init,
		hand_mouse_shutdown,
		nullptr,
		hand_mouse_update_frame,
		hand_mouse_update_poses },
	{ hand_system_none,
		1,
		[]() {return true;},
		[]() {},
		[]() {},
		nullptr,
		[]() {},
		[](bool) {} },
};
int32_t      hand_system = -1;
hand_state_t hand_state[2] = {};
float        hand_size_update = 0;
int32_t      input_hand_pointer_id[handed_max] = {-1, -1};

void input_hand_update_mesh(handed_ hand);

///////////////////////////////////////////

const hand_t *input_hand(handed_ hand) {
	return &hand_state[hand].info;
}

///////////////////////////////////////////

const controller_t *input_controller(handed_ hand) {
	return &input_controllers[hand];
}

button_state_ input_controller_menu() {
	return input_controller_menubtn;
}

///////////////////////////////////////////

hand_system_ input_hand_get_system() {
	return hand_sources[hand_system].system;
}

///////////////////////////////////////////

void input_hand_refresh_system() {
	int available_source = _countof(hand_sources) - 1;
	for (int32_t i = 0; i < _countof(hand_sources); i++) {
		if (hand_sources[i].available()) {
			available_source = i;
			break;
		}
	}
	if (available_source != hand_system) {
		hand_system = available_source;

		// Force the hand size to recalculate next update
		hand_size_update = 0;
	}
}

///////////////////////////////////////////

void modify(pose_t *pose, vec3 offset) {
	quat rot = quat_from_angles(0, 180, 0);
	for (int32_t i = 0; i < 25; i++) {
		pose[i].position   += offset;
		pose[i].position   *= 1.15f;
		pose[i].orientation = rot * pose[i].orientation;
	}
}

void input_hand_init() {
	input_hand_pointer_id[handed_left ] = input_add_pointer(input_source_hand | input_source_hand_left  | input_source_can_press);
	input_hand_pointer_id[handed_right] = input_add_pointer(input_source_hand | input_source_hand_right | input_source_can_press);

	float blend = 1;
	for (int32_t i = 0; i < _countof(hand_sources); i++) {
		if (hand_sources[i].system == hand_system_oxr_controllers) {
			blend = hand_sources[i].pinch_blend;
			break;
		}
	}
	vec3 from_pt = vec3_lerp(input_pose_neutral[0][4].position, input_pose_neutral[1][4].position, blend);
	vec3 grab_pt = vec3_lerp(input_pose_pinch  [0][4].position, input_pose_pinch  [1][4].position, blend);

	modify(&input_pose_fist   [0][0], {});
	modify(&input_pose_neutral[0][0], {});
	modify(&input_pose_point  [0][0], {});
	modify(&input_pose_pinch  [0][0], from_pt - grab_pt);

	material_t hand_mat = material_copy_id(default_id_material);
	material_set_id          (hand_mat, default_id_material_hand);
	material_set_transparency(hand_mat, transparency_blend);

	gradient_t color_grad = gradient_create();
	gradient_add(color_grad, color128{ .4f,.4f,.4f,0 }, 0.0f);
	gradient_add(color_grad, color128{ .6f,.6f,.6f,0 }, 0.4f);
	gradient_add(color_grad, color128{ .8f,.8f,.8f,1 }, 0.55f);
	gradient_add(color_grad, color128{ 1,  1,  1,  1 }, 1.0f);

	color32 gradient[16 * 16];
	for (int32_t y = 0; y < 16; y++) {
		color32 col = gradient_get32(color_grad, 1-y/15.f);
	for (int32_t x = 0; x < 16; x++) {
		gradient[x + y * 16] = col;
	} }
	gradient_release(color_grad);

	tex_t gradient_tex = tex_create(tex_type_image, tex_format_rgba32_linear);
	tex_set_colors (gradient_tex, 16, 16, gradient);
	tex_set_address(gradient_tex, tex_address_clamp);
	material_set_texture     (hand_mat, "diffuse", gradient_tex);
	material_set_queue_offset(hand_mat, 10);
	
	// Initialize the hands!
	for (int32_t i = 0; i < handed_max; i++) {
		hand_state[i].visible  = true;
		hand_state[i].solid    = true;
		hand_state[i].material = hand_mat;
		material_addref(hand_state[i].material);

		hand_state[i].info.palm.orientation = quat_identity;
		hand_state[i].info.handedness = (handed_)i;
		input_hand_update_mesh((handed_)i);

		hand_state[i].solids[0] = solid_create(vec3_zero, quat_identity, solid_type_unaffected);
		solid_add_box    (hand_state[i].solids[0], vec3{ 0.03f, .1f, .2f });
		solid_set_enabled(hand_state[i].solids[0], false);

		// Set up initial default hand pose, so we don't get any accidental pinch/grips on start
		memcpy(hand_state[i].pose_blend, input_pose_neutral, sizeof(pose_t) * SK_FINGERS * SK_FINGERJOINTS);
		hand_t &hand = hand_state[i].info;
		for (int32_t f = 0; f < 5; f++) {
		for (int32_t j = 0; j < 5; j++) {
			vec3 pos = input_pose_neutral[f][j].position;
			quat rot = input_pose_neutral[f][j].orientation;
			if (i == handed_right) {
				// mirror along x axis, our pose data is for left hand
				pos.x = -pos.x;
				rot.y = -rot.y;
				rot.z = -rot.z;
			}
			hand.fingers[f][j].position    = pos;
			hand.fingers[f][j].orientation = rot;
			hand.fingers[f][j].radius      = hand_finger_size[f] * hand_joint_size[j] * 0.35f;
		} }
	}

	tex_release(gradient_tex);
	material_release(hand_mat);

	for (int32_t i = 0; i < _countof(hand_sources); i++) {
		hand_sources[i].init();
	}

	input_hand_refresh_system();
}

///////////////////////////////////////////

void input_hand_shutdown() {
	for (int32_t i = 0; i < _countof(hand_sources); i++) {
		hand_sources[i].shutdown();
	}

	for (int32_t i = 0; i < handed_max; i++) {
		for (int32_t f = 0; f < SK_FINGER_SOLIDS; f++) {
			solid_release(hand_state[i].solids[f]);
		}
		material_release(hand_state[i].material);
		mesh_release    (hand_state[i].mesh.mesh);
		sk_free(hand_state[i].mesh.inds);
		sk_free(hand_state[i].mesh.verts);
	}
}

///////////////////////////////////////////

void input_hand_update() {
	hand_sources[hand_system].update_frame();

	// Update the hand size every second
	hand_size_update -= time_elapsedf();
	if (hand_size_update <= 0) {
		hand_size_update = 1;

		for (int32_t h = 0; h < handed_max; h++) {
			if (!(hand_state[h].info.tracked_state & button_state_active))
				continue;

			hand_state[h].info.size = 0;
			for (int32_t j = 0; j < hand_joint_tip-1; j++) {
				hand_state[h].info.size += vec3_magnitude(
					hand_state[h].info.fingers[hand_finger_middle][j  ].position -
					hand_state[h].info.fingers[hand_finger_middle][j+1].position);
			}
			hand_state[h].info.size += hand_state[h].info.fingers[hand_finger_middle][hand_joint_root].radius;
			hand_state[h].info.size += hand_state[h].info.fingers[hand_finger_middle][hand_joint_tip].radius;
		}
	}

	// Update hand states
	for (int32_t i = 0; i < handed_max; i++) {
		input_hand_state_update((handed_)i);
	}

	for (int32_t i = 0; i < _countof(hand_sources); i++) {
		if (hand_system != i && hand_sources[i].update_inactive != nullptr)
			hand_sources[i].update_inactive();
	}
}

///////////////////////////////////////////

void input_hand_update_poses(bool update_visuals) {
	if (hand_system >= 0)
		hand_sources[hand_system].update_poses(update_visuals);

	if (update_visuals) {
		for (int32_t i = 0; i < handed_max; i++) {
			// Update hand meshes, and draw 'em
			bool tracked = hand_state[i].info.tracked_state & button_state_active;
			if (hand_state[i].visible && hand_state[i].material != nullptr && tracked) {
				render_add_mesh(hand_state[i].mesh.mesh, hand_state[i].material, hand_state[i].mesh.root_transform, hand_state[i].info.pinch_state & button_state_active ? color128{3, 3, 3, 1} : color128{1,1,1,1});
			}

			// Update hand physics
			if (hand_state[i].solid) {
				solid_set_enabled(hand_state[i].solids[0], tracked);
				if (tracked) {
					solid_move(hand_state[i].solids[0], hand_state[i].info.palm.position, hand_state[i].info.palm.orientation);
				}
			}
		}
	}
}

///////////////////////////////////////////

void input_hand_update_meshes() {
	for (int32_t i = 0; i < handed_max; i++) {
		// Update hand meshes, and draw 'em
		bool tracked = hand_state[i].info.tracked_state & button_state_active;
		if (hand_state[i].visible && hand_state[i].material != nullptr && tracked) {
			input_hand_update_mesh((handed_)i);
		}
	}
}

///////////////////////////////////////////

void input_hand_state_update(handed_ handedness) {
	hand_t &hand = hand_state[handedness].info;

	// Update hand state based on inputs
	bool was_trigger = hand.pinch_state & button_state_active;
	bool was_gripped = hand.grip_state  & button_state_active;
	// Clear all except tracking state
	hand.pinch_state = button_state_inactive;
	hand.grip_state  = button_state_inactive;
	
	const float grip_activation_dist  = (was_gripped ? 6 : 5) * cm2m;
	const float pinch_activation_dist = (was_trigger ? 2 : 1) * cm2m;
	float finger_offset = hand.fingers[hand_finger_index][hand_joint_tip].radius + hand.fingers[hand_finger_thumb][hand_joint_tip].radius;
	float finger_dist   = vec3_magnitude((hand.fingers[hand_finger_index][hand_joint_tip].position - hand.fingers[hand_finger_thumb][hand_joint_tip].position)) - finger_offset;
	float grip_offset   = hand.fingers[hand_finger_ring][hand_joint_tip].radius + hand.fingers[hand_finger_ring][hand_joint_root].radius;
	float grip_dist     = vec3_magnitude(hand.fingers[hand_finger_ring][hand_joint_tip].position - hand.fingers[hand_finger_ring][hand_joint_root].position) - grip_offset;
	
	const float pinch_max = 8 * cm2m;
	const float grip_max  = 11 * cm2m;
	hand.pinch_activation = fminf(1, fmaxf(0, 1 - ((finger_dist - pinch_activation_dist) / (pinch_max - pinch_activation_dist))));
	hand.grip_activation  = fminf(1, fmaxf(0, 1 - ((grip_dist   - grip_activation_dist)  / (grip_max  - grip_activation_dist))));
	bool is_trigger = finger_dist <= pinch_activation_dist;
	bool is_grip    = grip_dist   <= grip_activation_dist;

	hand.pinch_pt = vec3_lerp(
		hand.fingers[0][4].position,
		hand.fingers[1][4].position,
		hand_sources[hand_system].pinch_blend);

	if (was_trigger != is_trigger) hand.pinch_state |= is_trigger ? button_state_just_active : button_state_just_inactive;
	if (was_gripped != is_grip)    hand.grip_state  |= is_grip    ? button_state_just_active : button_state_just_inactive;
	if (is_trigger) hand.pinch_state |= button_state_active;
	if (is_grip)    hand.grip_state  |= button_state_active;
}

///////////////////////////////////////////

hand_joint_t *input_hand_get_pose_buffer(handed_ hand) {
	return &hand_state[hand].info.fingers[0][0];
}

///////////////////////////////////////////

hand_mesh_t *input_hand_mesh_data(handed_ handedness) {
	return &hand_state[handedness].mesh;
}

///////////////////////////////////////////

void input_hand_sim_poses(handed_ handedness, bool mouse_adjustments, vec3 hand_pos, quat orientation) {
	hand_t &hand = hand_state[handedness].info;

	// only sim it if it's tracked
	if (!(hand.tracked_state & button_state_active))
		return;

	// For mice based hands, we change the hand's location to center the
	// pointer finger on the mouse
	vec3 finger_off = mouse_adjustments
		? hand_state[handedness].pose_blend[1][4].position
		: vec3_zero;

	// Update the position and orientation of each joint to be at its proper
	// location in world space
	for (int32_t f = 0; f < 5; f++) {
		const pose_t *finger = &hand_state[handedness].pose_blend[f][0];
	for (int32_t j = 0; j < 5; j++) {
		vec3 pos = finger[j].position - finger_off;
		quat rot = finger[j].orientation;
		if (handedness == handed_right) {
			// mirror along x axis, our pose data is for left hand
			pos.x = -pos.x;
			rot.y = -rot.y;
			rot.z = -rot.z;
		}
		hand.fingers[f][j].position    = orientation * pos + hand_pos;
		hand.fingers[f][j].orientation = rot * orientation;
		hand.fingers[f][j].radius      = hand_finger_size[f] * hand_joint_size[j] * 0.35f;
	} }

	// Update some of the higher level hand poses

	// OpenXR spec defines "The palm joint is located at the center of the
	// middle finger's metacarpal bone", so we'll use that for the position.
	hand.palm.position =
		(hand.fingers[2][0].position +
		 hand.fingers[2][1].position) * 0.5f;
	// However, OpenXR's facing direction seems... less practical, so here we
	// arrange them so that "forward" faces out from the palm.
	hand.palm.orientation = quat_from_angles(
		0,
		handedness == handed_right ?  90.f : -90.f,
		handedness == handed_right ? -90.f :  90.f) * orientation;
	hand.wrist.position    = (hand.fingers[1][0].position + hand.fingers[4][0].position) / 2;
	hand.wrist.orientation = hand.palm.orientation;
}

///////////////////////////////////////////

void input_hand_sim(handed_ handedness, bool center_on_finger, vec3 hand_pos, quat orientation, bool tracked, bool trigger_pressed, bool grip_pressed) {
	hand_t &hand = hand_state[handedness].info;

	// Update tracking state
	bool was_tracked = hand.tracked_state & button_state_active;
	hand.tracked_state = button_make_state(was_tracked, tracked);

	// only sim it if it's tracked
	if (!tracked)
		return;

	// Switch pose based on what buttons are pressed
	const pose_t *dest_pose;
	if      ( trigger_pressed && !grip_pressed) dest_pose = &input_pose_pinch  [0][0];
	else if ( trigger_pressed &&  grip_pressed) dest_pose = &input_pose_fist   [0][0];
	else if (!trigger_pressed &&  grip_pressed) dest_pose = &input_pose_point  [0][0];
	else                                        dest_pose = &input_pose_neutral[0][0];

	// Blend our active pose with our desired pose, for smooth transitions
	// between poses
	float delta = time_elapsedf_unscaled() * 30;
	delta = delta>1?1:delta;
	for (int32_t f = 0; f < 5; f++) {
	for (int32_t j = 0; j < 5; j++) {
		pose_t *p = &hand_state[handedness].pose_blend[f][j];
		p->position    = vec3_lerp (p->position,    dest_pose[f * 5 + j].position,    delta);
		p->orientation = quat_slerp(p->orientation, dest_pose[f * 5 + j].orientation, delta);
	} }

	// Update all the hand joints now that we have a pose to work from.
	input_hand_sim_poses(handedness, center_on_finger, hand_pos, orientation);
}

///////////////////////////////////////////

const vec3 sincos[] = { 
	{cosf(162*deg2rad), sinf(162*deg2rad), 0},
	{cosf(90 *deg2rad), sinf(90 *deg2rad), 0},
	{cosf(18 *deg2rad), sinf(18 *deg2rad), 0},
	{cosf(18 *deg2rad), sinf(18 *deg2rad), 0},
	{cosf(306*deg2rad), sinf(306*deg2rad), 0},
	{cosf(234*deg2rad), sinf(234*deg2rad), 0},
	{cosf(162*deg2rad), sinf(162*deg2rad), 0},};

const vec3 sincos_norm[] = { 
	{cosf(126*deg2rad), sinf(126*deg2rad), 0},
	{cosf(90 *deg2rad), sinf(90 *deg2rad), 0},
	{cosf(54 *deg2rad), sinf(54 *deg2rad), 0},
	{cosf(18 *deg2rad), sinf(18 *deg2rad), 0},
	{cosf(306*deg2rad), sinf(306*deg2rad), 0},
	{cosf(234*deg2rad), sinf(234*deg2rad), 0},
	{cosf(162*deg2rad), sinf(162*deg2rad), 0},};

const float texcoord_v[SK_FINGERJOINTS + 1] = { 1, 1-0.44f, 1-0.69f, 1-0.85f, 1-0.96f, 1-0.99f };

void input_hand_update_mesh(handed_ hand) {
	hand_mesh_t &data = hand_state[hand].mesh;

	const int32_t ring_count  = _countof(sincos);
	const int32_t slice_count = SK_FINGERJOINTS + 1;

	// if this mesh hasn't been initialized yet
	if (data.verts == nullptr) {
		data.root_transform = matrix_identity;
		data.vert_count = (_countof(sincos) * slice_count + 1) * SK_FINGERS ; // verts: per joint, per finger 
		data.ind_count  = (3 * 5 * 2 * (slice_count-1) + (8 * 3)) * (SK_FINGERS) ; // inds: per face, per connecting faces, per joint section, per finger, plus 2 caps
		data.verts      = sk_malloc_t(vert_t, data.vert_count);
		data.inds       = sk_malloc_t(vind_t, data.ind_count );

		int32_t ind = 0;
		for (vind_t f = 0; f < SK_FINGERS; f++) {
			vind_t start_vert =  f    * (ring_count * slice_count + 1);
			vind_t end_vert   = (f+1) * (ring_count * slice_count + 1) - (ring_count + 1);

			// start cap
			data.inds[ind++] = start_vert+2;
			data.inds[ind++] = start_vert+1;
			data.inds[ind++] = start_vert+0;

			data.inds[ind++] = start_vert+4;
			data.inds[ind++] = start_vert+3;
			data.inds[ind++] = start_vert+6;

			data.inds[ind++] = start_vert+5;
			data.inds[ind++] = start_vert+4;
			data.inds[ind++] = start_vert+6;
		
			// tube faces
			for (vind_t j = 0; j < slice_count-1; j++) {
			for (vind_t c = 0; c < ring_count-1; c++) {
				if (c == 2) c++;
				vind_t curr1 = start_vert +  j    * ring_count + c;
				vind_t next1 = start_vert + (j+1) * ring_count + c;
				vind_t curr2 = start_vert +  j    * ring_count + (c+1);
				vind_t next2 = start_vert + (j+1) * ring_count + (c+1);
				data.inds[ind++] = next2;
				data.inds[ind++] = next1;
				data.inds[ind++] = curr1;

				data.inds[ind++] = curr2;
				data.inds[ind++] = next2;
				data.inds[ind++] = curr1;
			} }

			// end cap
			data.inds[ind++] = end_vert+0;
			data.inds[ind++] = end_vert+1;
			data.inds[ind++] = end_vert+7;

			data.inds[ind++] = end_vert+1;
			data.inds[ind++] = end_vert+2;
			data.inds[ind++] = end_vert+7;

			data.inds[ind++] = end_vert+3;
			data.inds[ind++] = end_vert+4;
			data.inds[ind++] = end_vert+7;

			data.inds[ind++] = end_vert+4;
			data.inds[ind++] = end_vert+5;
			data.inds[ind++] = end_vert+7;

			data.inds[ind++] = end_vert+5;
			data.inds[ind++] = end_vert+6;
			data.inds[ind++] = end_vert+7;
		}

		// Generate uvs and colors for the mesh
		int v = 0;
		for (int f = 0; f < SK_FINGERS; f++) {
			float x = ((float)f / SK_FINGERS) + (0.5f/SK_FINGERS);
		for (int j = 0; j < slice_count; j++) {
			float y = texcoord_v[f==0 ? maxi(0,j-1) : j];
			
			data.verts[v  ].uv  = { x,y };
			data.verts[v++].col = { 255,255,255,255 };
			data.verts[v  ].uv  = { x,y };
			data.verts[v++].col = { 255,255,255,255 };
			data.verts[v  ].uv  = { x,y };
			data.verts[v++].col = { 255,255,255,255 };
			data.verts[v  ].uv  = { x,y };
			data.verts[v++].col = { 200,200,200,255 };
			data.verts[v  ].uv  = { x,y };
			data.verts[v++].col = { 200,200,200,255 };
			data.verts[v  ].uv  = { x,y };
			data.verts[v++].col = { 200,200,200,255 };
			data.verts[v  ].uv  = { x,y };
			data.verts[v++].col = { 200,200,200,255 };
		} 
		data.verts[v  ].uv  = { x,0 };
		data.verts[v++].col = { 255,255,255,255 };
		}

		data.mesh = mesh_create();
		mesh_set_keep_data(data.mesh, false);
		mesh_set_id(data.mesh, hand == handed_left
			? default_id_mesh_lefthand
			: default_id_mesh_righthand);
		mesh_set_inds(data.mesh, data.inds, data.ind_count);
	}

	int v = 0;
	for (int f = 0; f < SK_FINGERS;      f++) {
		const hand_joint_t &pose_last = hand_state[hand].info.fingers[f][SK_FINGERJOINTS-1];
		vec3 tip_fwd = pose_last.orientation * vec3_forward;
		vec3 tip_up  = pose_last.orientation * vec3_up;
		for (int j = 0; j < SK_FINGERJOINTS; j++) {
			const hand_joint_t &pose_prev = hand_state[hand].info.fingers[f][maxi(0,j-1)];
			const hand_joint_t &pose      = hand_state[hand].info.fingers[f][j];
			quat orientation = quat_slerp(pose_prev.orientation, pose.orientation, 0.5f);

			// Make local right and up axis vectors
			vec3  right = orientation * vec3_right;
			vec3  up    = orientation * vec3_up;

			// Find the scale for this joint
			float scale = pose.radius;
			if (f == 0 && j < 2) scale *= 0.5f; // thumb is too fat at the bottom

			// Use the local axis to create a ring of verts
			for (int32_t i = 0; i < ring_count; i++) {
				data.verts[v].norm = (up*sincos_norm[i].y + right*sincos_norm[i].x) * SK_SQRT2;
				data.verts[v].pos  = pose.position + (up*sincos[i].y + right*sincos[i].x)*scale;
				v++;
			}

			// Last ring to blunt the fingertip
			if (j == SK_FINGERJOINTS - 1) {
				scale = scale * 0.75f;
				for (int32_t i = 0; i < ring_count; i++) {
					vec3 at = pose.position + tip_fwd * pose.radius * 0.65f;
					data.verts[v].norm = (up*sincos_norm[i].y + right*sincos_norm[i].x) * SK_SQRT2;
					data.verts[v].pos  = at + (up*sincos[i].y + right*sincos[i].x)*scale + tip_up * pose.radius*0.25f;
					v++;
				}
			}
		}
		data.verts[v].norm = tip_fwd;
		data.verts[v].pos  = pose_last.position + data.verts[v].norm * pose_last.radius + tip_up * pose_last.radius * 0.9f;
		v++;
	}

	// And update the mesh vertices!
	mesh_set_verts(data.mesh, data.verts, data.vert_count);
}

///////////////////////////////////////////

void input_hand_visible(handed_ hand, bool32_t visible) {
	if (hand == handed_max) {
		input_hand_visible(handed_left,  visible);
		input_hand_visible(handed_right, visible);
		return;
	}

	hand_state[hand].visible = visible;
}

///////////////////////////////////////////

void input_hand_solid(handed_ hand, bool32_t solid) {
	if (hand == handed_max) {
		input_hand_solid(handed_left,  solid);
		input_hand_solid(handed_right, solid);
		return;
	}

	hand_state[hand].solid = solid;
	for (int32_t i = 0; i < SK_FINGER_SOLIDS; i++) {
		solid_set_enabled(hand_state[hand].solids[i], solid);
	}
}

///////////////////////////////////////////

void input_hand_material(handed_ hand, material_t material) {
	if (hand == handed_max) {
		input_hand_material(handed_left,  material);
		input_hand_material(handed_right, material);
		return;
	}

	material_release(hand_state[hand].material);
	
	if (material != nullptr)
		material_addref(material);

	hand_state[hand].material = material;
}

} // namespace sk