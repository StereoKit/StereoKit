#include "input_hand.h"
#include "input_hand_poses.h"

#include "assets.h"
#include "material.h"

#define SK_FINGERS 5
#define SK_FINGERJOINTS 5
#define SK_SQRT2 1.41421356237f

pose_t hand_pose_blend[2][5][5];

bool        hand_visible[2] = { true, true };
material_t  hand_material[2] = { };
hand_t      hand_info[2] = {};
hand_mesh_t hand_mesh[2] = { {},{} };
const float hand_joint_size [5] = {.01f,.026f,.023f,.02f,.015f}; // in order of hand_joint_. found by measuring the width of my pointer finger when flattened on a ruler
const float hand_finger_size[5] = {1.15f,1,1,.85f,.75f}; // in order of hand_finger_. Found by comparing the distal joint of my index finger, with my other distal joints
transform_t hand_transform;

void input_hand_init() {
	hand_material[handed_left ] = material_find("default/material");
	hand_material[handed_right] = material_find("default/material");
	transform_initialize(hand_transform);

	// Initialize the hand mesh at startup, don't pay creation costs later!
	hand_info[0].handedness = handed_left;
	hand_info[1].handedness = handed_right;
	input_hand_update_mesh(hand_info[0]);
	input_hand_update_mesh(hand_info[1]);
}

void input_hand_shutdown() {
	for (size_t i = 0; i < handed_max; i++) {
		if (hand_material[i]   != nullptr) material_release(hand_material[i]);
		if (hand_mesh[i].mesh  != nullptr) mesh_release(hand_mesh[i].mesh);
		if (hand_mesh[i].inds  != nullptr) free(hand_mesh[i].inds);
		if (hand_mesh[i].verts != nullptr) free(hand_mesh[i].verts);
	}
}

void input_hand_update() {
	// Update hand meshes
	for (size_t i = 0; i < handed_max; i++) {
		if (hand_visible[i] && hand_material[i] != nullptr && hand_info[i].state & input_state_tracked) {
			input_hand_update_mesh(hand_info[i]);
			render_add_mesh(hand_mesh[i].mesh, hand_material[i], hand_transform);
		}
	}
}

void input_hand_sim(handed_ handedness, const vec3 &hand_pos, const quat &orientation, bool tracked, bool trigger_pressed, bool grip_pressed) {
	hand_t &hand = hand_info[handedness];
	hand.root.position    = hand_pos;
	hand.root.orientation = orientation;
	
	// Update hand state based on inputs
	bool was_tracked = hand.state & input_state_tracked;
	bool was_trigger = hand.state & input_state_pinch;
	bool was_gripped = hand.state & input_state_grip;
	hand.state = input_state_none;

	if (was_tracked != tracked)         hand.state |= tracked         ? input_state_justtracked : input_state_untracked;
	if (was_trigger != trigger_pressed) hand.state |= trigger_pressed ? input_state_justpinch   : input_state_unpinch;
	if (was_gripped != grip_pressed)    hand.state |= grip_pressed    ? input_state_justgrip    : input_state_ungrip;
	if (tracked)         hand.state |= input_state_tracked;
	if (trigger_pressed) hand.state |= input_state_pinch;
	if (grip_pressed)    hand.state |= input_state_grip;

	// only sim it if it's tracked
	if (tracked) {
		// Switch pose based on what buttons are pressed
		const pose_t *dest_pose;
		if (trigger_pressed && !grip_pressed)
			dest_pose = &input_pose_pinch[0][0];
		else if (trigger_pressed && grip_pressed)
			dest_pose = &input_pose_fist[0][0];
		else if (!trigger_pressed && grip_pressed)
			dest_pose = &input_pose_point[0][0];
		else
			dest_pose = &input_pose_neutral[0][0];

		// Blend our active pose with our desired pose, for smooth transitions
		// between poses
		float delta = sk_time_elapsedf() * 20;
		delta = delta>1?1:delta;
		for (size_t f = 0; f < 5; f++) {
		for (size_t j = 0; j < 5; j++) {
			pose_t *p = &hand_pose_blend[handedness][f][j];
			p->position    = vec3_lerp(p->position,    dest_pose[f * 5 + j].position,    delta);
			p->orientation = quat_lerp(p->orientation, dest_pose[f * 5 + j].orientation, delta);
		} }
	
		for (size_t f = 0; f < 5; f++) {
			const pose_t *finger = &hand_pose_blend[handedness][f][0];
		for (size_t j = 0; j < 5; j++) {
			vec3 pos = finger[j].position;
			quat rot = finger[j].orientation;
			if (handedness == handed_right) {
				// mirror along x axis, our pose data is for left hand
				pos.x = -pos.x;
				rot.j = -rot.j;
				rot.k = -rot.k;
			}
			hand.fingers[f][j].position    = orientation * pos + hand_pos;
			hand.fingers[f][j].orientation = rot * orientation;
		} }
	}
}

void input_hand_update_mesh(const hand_t &hand) {
	hand_mesh_t &data = hand_mesh[hand.handedness];

	// if this mesh hasn't been initialized yet
	if (data.verts == nullptr) {
		data.vert_count = 4 * SK_FINGERJOINTS * SK_FINGERS; // verts: per joint, per finger 
		data.ind_count  = (3 * 8 * (SK_FINGERJOINTS-1) + (4 * 3)) * (SK_FINGERS) ; // inds: per face, per connecting faces, per joint section, per finger, plus 2 caps
		data.verts      = (vert_t  *)malloc(sizeof(vert_t  ) * data.vert_count);
		data.inds       = (uint16_t*)malloc(sizeof(uint16_t) * data.ind_count );

		int ind = 0;
		for (int f = 0; f < SK_FINGERS; f++) {
			int start_vert =  f    * 4 * SK_FINGERJOINTS;
			int end_vert   = (f+1) * 4 * SK_FINGERJOINTS;

			// start cap
			data.inds[ind++] = start_vert+0;
			data.inds[ind++] = start_vert+1;
			data.inds[ind++] = start_vert+2;

			data.inds[ind++] = start_vert+0;
			data.inds[ind++] = start_vert+2;
			data.inds[ind++] = start_vert+3;
		
			// tube faces
			for (int j = 0; j < SK_FINGERJOINTS-1; j++) {
			for (int c = 0; c < 4; c++) {
				int curr1 = start_vert +  j    * 4 + c;
				int next1 = start_vert + (j+1) * 4 + c;
				int curr2 = start_vert +  j    * 4 + (c+1)%4;
				int next2 = start_vert + (j+1) * 4 + (c+1)%4;
				data.inds[ind++] = curr1;
				data.inds[ind++] = next1;
				data.inds[ind++] = next2;

				data.inds[ind++] = curr1;
				data.inds[ind++] = next2;
				data.inds[ind++] = curr2;
			} }

			// end cap
			data.inds[ind++] = end_vert-2;
			data.inds[ind++] = end_vert-3;
			data.inds[ind++] = end_vert-4;

			data.inds[ind++] = end_vert-1;
			data.inds[ind++] = end_vert-2;
			data.inds[ind++] = end_vert-4;
		}

		// Generate uvs and colors for the mesh
		int v = 0;
		for (int f = 0; f < SK_FINGERS;      f++) {
		for (int j = 0; j < SK_FINGERJOINTS; j++) {
			float y = j / (float)(SK_FINGERJOINTS-1);
			data.verts[v].uv  = { 0,y };
			data.verts[v].col = { 255,255,255,255 };
			v++;
			data.verts[v].uv  = { .5f,y };
			data.verts[v].col = { 255,255,255,255 };
			v++;
			data.verts[v].uv  = { 1,y };
			data.verts[v].col = { 255,255,255,255 };
			v++;
			data.verts[v].uv  = { .5f,y };
			data.verts[v].col = { 255,255,255,255 };
			v++;
		} }

		if (hand.handedness == handed_left)
			data.mesh = mesh_create("default/mesh_lefthand");
		else
			data.mesh = mesh_create("default/mesh_righthand");
		mesh_set_inds(data.mesh, data.inds, data.ind_count);
	}

	int v = 0;
	for (int f = 0; f < SK_FINGERS;      f++) {
	for (int j = 0; j < SK_FINGERJOINTS; j++) {
		const pose_t &pose = hand.fingers[f][j];

		// Make local right and up axis vectors
		vec3  right = pose.orientation * vec3{ 1,0,0 };
		vec3  up    = pose.orientation * vec3{ 0,1,0 };

		// Find the scale for this joint
		float scale = hand_finger_size[f] * hand_joint_size[j] * 0.25f;
		if (f == 0 && j < 2) scale *= 0.5f; // thumb is too fat at the bottom

		// Use the local axis to create a ring of verts
		data.verts[v].norm = (up - right) * SK_SQRT2;
		data.verts[v].pos  = pose.position + data.verts[v].norm*scale;
		v++;
		data.verts[v].norm = (right + up) * SK_SQRT2;
		data.verts[v].pos  = pose.position + data.verts[v].norm*scale;
		v++;
		data.verts[v].norm = (right - up) * SK_SQRT2;
		data.verts[v].pos  = pose.position + data.verts[v].norm*scale;
		v++;
		data.verts[v].norm = (vec3{ 0,0,0 } - up - right) * SK_SQRT2;
		data.verts[v].pos  = pose.position + data.verts[v].norm*scale;
		v++;
	} }

	// And update the mesh vertices!
	mesh_set_verts(data.mesh, data.verts, data.vert_count);
}

void input_hand_visible(handed_ hand, int visible) {
	hand_visible[hand] = visible;
}
void input_hand_material(handed_ hand, material_t material) {
	if (hand_material[hand] != nullptr) 
		material_release(hand_material[hand]);
	
	if (material != nullptr)
		assets_addref(material->header);

	hand_material[hand] = material;
}