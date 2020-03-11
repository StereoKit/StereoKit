#include "../../stereokit.h"
#include "../input.h"
#include "input_hand.h"
#include "hand_poses.h"

#include "hand_mouse.h"
#include "hand_mirage.h"
#include "hand_leap.h"
#include "hand_override.h"
#include "hand_oxr_controller.h"
#include "hand_oxr_articulated.h"

#include "../../asset_types/assets.h"
#include "../../asset_types/material.h"

namespace sk {

///////////////////////////////////////////

#define SK_FINGERS 5
#define SK_FINGERJOINTS 5
#define SK_FINGER_SOLIDS 1

struct hand_mesh_t {
	mesh_t  mesh;
	vert_t *verts;
	int     vert_count;
	vind_t *inds;
	int     ind_count;
};

struct hand_state_t {
	hand_t      info;
	pose_t      pose_blend[5][5];
	solid_t     solids[SK_FINGER_SOLIDS];
	material_t  material;
	hand_mesh_t mesh;
	bool        visible;
	bool        enabled;
};

struct hand_system_t {
	hand_system_ system;
	bool         initialized;
	bool (*available)()        = nullptr;
	void (*init)()             = nullptr;
	void (*shutdown)()         = nullptr;
	void (*update_frame)()     = nullptr;
	void (*update_predicted)() = nullptr;
};

hand_system_t hand_sources[] = { // In order of priority
	{ hand_system_override, false,
		hand_override_available,
		hand_override_init,
		hand_override_shutdown,
		hand_override_update_frame,
		hand_override_update_predicted },
	{ hand_system_oxr_articulated, false,
		hand_oxra_available,
		hand_oxra_init,
		hand_oxra_shutdown,
		hand_oxra_update_frame,
		hand_oxra_update_predicted },
	{ hand_system_mirage, false,
		hand_mirage_available,
		hand_mirage_init,
		hand_mirage_shutdown,
		hand_mirage_update_frame,
		hand_mirage_update_predicted },
#ifndef SK_NO_LEAP_MOTION
	{ hand_system_leap, false,
		hand_leap_present,
		hand_leap_init,
		hand_leap_shutdown,
		hand_leap_update_frame,
		hand_leap_update_predicted },
#endif
	{ hand_system_oxr_controllers, false,
		hand_oxrc_available,
		hand_oxrc_init,
		hand_oxrc_shutdown,
		hand_oxrc_update_frame,
		hand_oxrc_update_predicted },
	{ hand_system_mouse, false,
		hand_mouse_available,
		hand_mouse_init,
		hand_mouse_shutdown,
		hand_mouse_update_frame,
		hand_mouse_update_predicted },
	{ hand_system_none, false,
		[]() {return true;},
		[]() {},
		[]() {},
		[]() {},
		[]() {} },
};
int32_t      hand_system = -1;
hand_state_t hand_state[2];

void input_hand_update_mesh(handed_ hand);

///////////////////////////////////////////

const hand_t &input_hand(handed_ hand) {
	return hand_state[hand].info;
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
		// TODO: Only really necessary for oxrc -> mirage transition, when mirage dies, this can go away
		if (hand_system >= 0) {
			hand_sources[hand_system].shutdown();
			hand_sources[hand_system].initialized = false;
		}
		hand_system = available_source;

		const char *source_name = "N/A";
		switch (hand_sources[hand_system].system) {
		case hand_system_leap:            source_name = "Leap"; break;
		case hand_system_mirage:          source_name = "UWP Articulated"; break;
		case hand_system_mouse:           source_name = "Mouse"; break;
		case hand_system_none:            source_name = "None"; break;
		case hand_system_override:        source_name = "Override"; break;
		case hand_system_oxr_articulated: source_name = "OpenXR Articulated"; break;
		case hand_system_oxr_controllers: source_name = "OpenXR Controllers"; break;
		}

		log_diagf("Switched to input source: %s", source_name);
		if (!hand_sources[hand_system].initialized) {
			hand_sources [hand_system].init();
			hand_sources [hand_system].initialized = true;
		}
	}
}

///////////////////////////////////////////

void modify(pose_t *pose, vec3 offset) {
	quat rot = quat_from_angles(0, 180, 0);
	for (size_t i = 0; i < 25; i++) {
		pose[i].position   += offset;
		pose[i].position   *= 1.15f;
		pose[i].orientation = rot * pose[i].orientation;
	}
}

void input_hand_init() {
	modify(&input_pose_fist   [0][0], {});
	modify(&input_pose_neutral[0][0], {});
	modify(&input_pose_point  [0][0], {});
	modify(&input_pose_pinch  [0][0], 
		(vec3{ 0.02675417f,0.02690793f,-0.07531749f }-vec3{0.04969539f,0.02166998f,-0.0236005f}) * (sk_active_runtime() == runtime_flatscreen ? 1 : 0.5f));

	material_t hand_mat = material_copy_id("default/material");
	material_set_id          (hand_mat, "default/material_hand");
	material_set_transparency(hand_mat, transparency_blend);

	gradient_t color_grad = gradient_create();
	gradient_add(color_grad, color128{ 1,1,1,0 }, 0.0f);
	gradient_add(color_grad, color128{ 1,1,1,0 }, 0.2f);
	gradient_add(color_grad, color128{ 1,1,1,1 }, 0.9f);

	color32 gradient[16 * 16];
	for (int32_t y = 0; y < 16; y++) {
		color32 col = gradient_get32(color_grad, y/15.f);
	for (int32_t x = 0; x < 16; x++) {
		gradient[x + y * 16] = col;
	} }
	gradient_release(color_grad);

	tex_t gradient_tex = tex_create();
	tex_set_colors (gradient_tex, 16, 16, gradient);
	tex_set_address(gradient_tex, tex_address_clamp);
	material_set_texture     (hand_mat, "diffuse", gradient_tex);
	material_set_queue_offset(hand_mat, -10);
	
	// Initialize the hands!
	for (size_t i = 0; i < handed_max; i++) {
		hand_state[i].visible  = true;
		hand_state[i].material = hand_mat;
		assets_addref(hand_state[i].material->header);

		hand_state[i].info.palm.orientation = quat_identity;
		hand_state[i].info.handedness = (handed_)i;
		input_hand_update_mesh((handed_)i);

		hand_state[i].solids[0] = solid_create(vec3_zero, quat_identity, solid_type_unaffected);
		solid_add_box    (hand_state[i].solids[0], vec3{ 0.03f, .1f, .2f });
		solid_set_enabled(hand_state[i].solids[0], false);

		// Set up initial default hand pose, so we don't get any accidental pinch/grips on start
		memcpy(hand_state[i].pose_blend, input_pose_neutral, sizeof(pose_t) * SK_FINGERS * SK_FINGERJOINTS);
		hand_t &hand = hand_state[i].info;
		for (size_t f = 0; f < 5; f++) {
		for (size_t j = 0; j < 5; j++) {
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
			hand.fingers[f][j].radius      = hand_finger_size[f] * hand_joint_size[j] * 0.5f;
		} }
	}

	tex_release(gradient_tex);
	material_release(hand_mat);

	input_hand_refresh_system();
}

///////////////////////////////////////////

void input_hand_shutdown() {
	for (size_t i = 0; i < _countof(hand_sources); i++) {
		if (hand_sources[i].initialized)
			hand_sources[i].shutdown();
		hand_sources[i].initialized = false;
	}

	for (size_t i = 0; i < handed_max; i++) {
		for (size_t f = 0; f < SK_FINGER_SOLIDS; f++) {
			solid_release(hand_state[i].solids[f]);
		}
		material_release(hand_state[i].material);
		mesh_release    (hand_state[i].mesh.mesh);
		free(hand_state[i].mesh.inds);
		free(hand_state[i].mesh.verts);
	}
}

///////////////////////////////////////////

void input_hand_update() {
	hand_sources[hand_system].update_frame();

	for (size_t i = 0; i < handed_max; i++) {
		// Update hand states
		input_hand_state_update((handed_)i);

		// Update hand meshes, and draw 'em
		bool tracked = hand_state[i].info.tracked_state & button_state_active;
		if (hand_state[i].visible && hand_state[i].material != nullptr && tracked) {
			render_add_mesh(hand_state[i].mesh.mesh, hand_state[i].material, matrix_identity, hand_state[i].info.pinch_state & button_state_active ? color128{3, 3, 3, 1} : color128{1,1,1,1});
		}

		// Update hand physics
		solid_set_enabled(hand_state[i].solids[0], tracked);
		if (tracked) {
			solid_move(hand_state[i].solids[0], hand_state[i].info.palm.position, hand_state[i].info.palm.orientation);
		}
	}
}

///////////////////////////////////////////

void input_hand_update_predicted() {
	hand_sources[hand_system].update_predicted();
}

///////////////////////////////////////////

void input_hand_update_meshes() {
	for (size_t i = 0; i < handed_max; i++) {
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

void input_hand_sim(handed_ handedness, const vec3 &hand_pos, const quat &orientation, bool tracked, bool trigger_pressed, bool grip_pressed) {
	hand_t &hand = hand_state[handedness].info;
	hand.palm.position    = hand_pos;
	hand.palm.orientation = quat_from_angles(
		0, 
		handedness == handed_right ?  90.f : -90.f, 
		handedness == handed_right ? -90.f :  90.f) * orientation;
	
	// Update hand state based on inputs
	bool was_tracked = hand.tracked_state & button_state_active;
	hand.tracked_state = button_make_state(was_tracked, tracked);

	// only sim it if it's tracked
	if (tracked) {
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
		for (size_t f = 0; f < 5; f++) {
		for (size_t j = 0; j < 5; j++) {
			pose_t *p = &hand_state[handedness].pose_blend[f][j];
			p->position    = vec3_lerp (p->position,    dest_pose[f * 5 + j].position,    delta);
			p->orientation = quat_slerp(p->orientation, dest_pose[f * 5 + j].orientation, delta);
		} }
	
		for (size_t f = 0; f < 5; f++) {
			const pose_t *finger = &hand_state[handedness].pose_blend[f][0];
		for (size_t j = 0; j < 5; j++) {
			vec3 pos = finger[j].position;
			quat rot = finger[j].orientation;
			if (handedness == handed_right) {
				// mirror along x axis, our pose data is for left hand
				pos.x = -pos.x;
				rot.y = -rot.y;
				rot.z = -rot.z;
			}
			hand.fingers[f][j].position    = orientation * pos + hand_pos;
			hand.fingers[f][j].orientation = rot * orientation;
			hand.fingers[f][j].radius      = hand_finger_size[f] * hand_joint_size[j] * 0.5f;
		} }
	}
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

void input_hand_update_mesh(handed_ hand) {
	hand_mesh_t &data = hand_state[hand].mesh;

	const int32_t ring_count = _countof(sincos);

	// if this mesh hasn't been initialized yet
	if (data.verts == nullptr) {
		data.vert_count = (_countof(sincos) * SK_FINGERJOINTS + 1) * SK_FINGERS ; // verts: per joint, per finger 
		data.ind_count  = (3 * 5 * 2 * (SK_FINGERJOINTS-1) + (8 * 3)) * (SK_FINGERS) ; // inds: per face, per connecting faces, per joint section, per finger, plus 2 caps
		data.verts      = (vert_t*)malloc(sizeof(vert_t) * data.vert_count);
		data.inds       = (vind_t*)malloc(sizeof(vind_t) * data.ind_count );

		int32_t ind = 0;
		for (vind_t f = 0; f < SK_FINGERS; f++) {
			vind_t start_vert =  f    * (ring_count * SK_FINGERJOINTS + 1);
			vind_t end_vert   = (f+1) * (ring_count * SK_FINGERJOINTS + 1) - (ring_count + 1);

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
			for (vind_t j = 0; j < SK_FINGERJOINTS-1; j++) {
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
		for (int f = 0; f < SK_FINGERS;      f++) {
			float x = ((float)f / SK_FINGERS) + (0.5f/SK_FINGERJOINTS);
		for (int j = 0; j < SK_FINGERJOINTS; j++) {
			float y = f == 0 
				? (fmaxf(0,(float)j-1) / (float)(SK_FINGERJOINTS-2)) 
				: (j / (float)(SK_FINGERJOINTS-1));
			
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
			data.verts[v  ].uv  = { x,1.0f };
			data.verts[v++].col = { 255,255,255,255 };
		}

		data.mesh = mesh_create();
		mesh_set_id(data.mesh, hand == handed_left
			? "default/mesh_lefthand"
			: "default/mesh_righthand");
		mesh_set_inds(data.mesh, data.inds, data.ind_count);
	}

	int v = 0;
	for (int f = 0; f < SK_FINGERS;      f++) {
	for (int j = 0; j < SK_FINGERJOINTS; j++) {
		const hand_joint_t &pose_prev = hand_state[hand].info.fingers[f][max(0,j-1)];
		const hand_joint_t &pose      = hand_state[hand].info.fingers[f][j];
		quat orientation = quat_slerp(pose_prev.orientation, pose.orientation, 0.5f);

		// Make local right and up axis vectors
		vec3  right = orientation * vec3_right;
		vec3  up    = orientation * vec3_up;

		// Find the scale for this joint
		float scale = pose.radius;
		if (f == 0 && j < 2) scale *= 0.5f; // thumb is too fat at the bottom

		// Use the local axis to create a ring of verts
		for (size_t i = 0; i < ring_count; i++) {
			data.verts[v].norm = up*sincos_norm[i].y + right*sincos_norm[i].x;
			data.verts[v].pos  = pose.position + (up*sincos[i].y + right*sincos[i].x)*scale;
			v++;
		}
	}
	const hand_joint_t &pose_prev = hand_state[hand].info.fingers[f][SK_FINGERJOINTS-2];
	const hand_joint_t &pose_last = hand_state[hand].info.fingers[f][SK_FINGERJOINTS-1];
	data.verts[v].norm = vec3_normalize(pose_last.position - pose_prev.position);
	data.verts[v].pos  = pose_last.position + data.verts[v].norm * pose_last.radius;
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

	for (size_t i = 0; i < SK_FINGER_SOLIDS; i++) {
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
		assets_addref(material->header);

	hand_state[hand].material = material;
}

} // namespace sk