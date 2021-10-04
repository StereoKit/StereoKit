#include "animation.h"
#include "model.h"

namespace sk {

array_t<model_t> animation_list = {};

///////////////////////////////////////////

int32_t anim_frame(const anim_curve_t *curve, int32_t prev_index, float t) {
	if (prev_index >= 0) {
		if (                                          curve->keyframe_times[prev_index  ] <= t && curve->keyframe_times[prev_index+1] > t) return prev_index;
		if (prev_index + 2 < curve->keyframe_count && curve->keyframe_times[prev_index+1] <= t && curve->keyframe_times[prev_index+2] > t) return prev_index + 1;
	}
	if (t < curve->keyframe_times[0]) return 0;

	int32_t l = 0, r = curve->keyframe_count - 1;
	while (l <= r) {
		int32_t mid = (l+r) / 2;
		if      (curve->keyframe_times[mid] <= t && curve->keyframe_times[mid+1] > t) return mid;
		else if (curve->keyframe_times[mid] < t) l = mid + 1;
		else if (curve->keyframe_times[mid] > t) r = mid - 1;
		else log_err("Weird comparison");
	}
	log_err("uhh");
	return -1;
}

///////////////////////////////////////////

vec3 anim_curve_sample_f3(const anim_curve_t *curve, int32_t *prev_index, float t) {
	int32_t frame = anim_frame(curve, *prev_index, t);
	float   pct   = (t - curve->keyframe_times[frame]) / (curve->keyframe_times[frame + 1] - curve->keyframe_times[frame]);
	vec3   *pts   = (vec3*)curve->keyframe_values;
	*prev_index = frame;

	switch (curve->interpolation) {
	case anim_interpolation_linear: return vec3_lerp(pts[frame], pts[frame + 1], pct); break;
	case anim_interpolation_step:   return pts[frame]; break;
	case anim_interpolation_cubic:  return vec3_lerp(pts[frame], pts[frame + 1], pct); break;
	default: return pts[frame]; break;
	}
}

///////////////////////////////////////////

quat anim_curve_sample_f4(const anim_curve_t *curve, int32_t *prev_index, float t) {
	int32_t frame = anim_frame(curve, *prev_index, t);
	float   pct   = (t - curve->keyframe_times[frame]) / (curve->keyframe_times[frame + 1] - curve->keyframe_times[frame]);
	quat   *pts   = (quat*)curve->keyframe_values;
	*prev_index = frame;

	switch (curve->interpolation) {
	case anim_interpolation_linear: return quat_slerp(pts[frame], pts[frame + 1], pct); break;
	case anim_interpolation_step:   return pts[frame]; break;
	case anim_interpolation_cubic:  return quat_slerp(pts[frame], pts[frame + 1], pct); break;
	default: return pts[frame]; break;
	}
}

///////////////////////////////////////////

static void anim_update_transforms(model_t model, model_node_id node, bool dirty_world) {
	while (node != -1) {
		// Only update this node if the animation touched the transform
		bool dirty = dirty_world;
		anim_transform_t *tr = &model->anim_inst.node_transforms[node];
		if (tr->dirty) {
			model->nodes[node].transform_local = matrix_trs(tr->translation, tr->rotation, tr->scale);
			tr->dirty = false;
			dirty     = true;
		}

		// If this node or a parent node was touched, we need to update the world
		// transform.
		if (dirty == true) {
			if (model->nodes[node].parent >= 0)
				model->nodes[node].transform_model = model->nodes[node].transform_local * model->nodes[model->nodes[node].parent].transform_model;
			else
				model->nodes[node].transform_model = model->nodes[node].transform_local;

			if (model->nodes[node].visual >= 0)
				model->visuals[model->nodes[node].visual].transform_model = model->nodes[node].transform_model;
		}
		/*vec3 p1 = matrix_transform_pt (model->nodes[node].transform_model, vec3_zero);
		vec3 p2 = matrix_transform_dir(model->nodes[node].transform_model, vec3_forward);
		vec3 p3 = matrix_transform_dir(model->nodes[node].transform_model, vec3_right);
		vec3 p4 = matrix_transform_dir(model->nodes[node].transform_model, vec3_up);
		line_add(p1, p1 + p2*0.5f, { 0,0,255,255 }, {255,255,255,255}, 0.01f);
		line_add(p1, p1 + p3*0.5f, { 255,0,0,255 }, {255,255,255,255}, 0.01f);
		line_add(p1, p1 + p4*0.5f, { 0,255,0,255 }, {255,255,255,255}, 0.01f);
		text_add_at(model->nodes[node].name, model->nodes[node].transform_model, 0, text_align_center_left); */

		anim_update_transforms(model, model->nodes[node].child, dirty);
		node = model->nodes[node].sibling;
	}
}

///////////////////////////////////////////

void anim_update(model_t model) {
	if (model->anim_inst.anim_id < 0) return;

	// Don't update more than once per frame
	float curr_time = time_getf();
	if (model->anim_inst.last_update == curr_time) return;
	model->anim_inst.last_update = curr_time;

	anim_t *anim = &model->anim_data.anims[model->anim_inst.anim_id];
	float   time = fmodf(curr_time-model->anim_inst.start_time, anim->duration);

	for (size_t i = 0; i < anim->curves.count; i++) {
		model_node_id node = anim->curves[i].node_id;

		model->anim_inst.node_transforms[node].dirty = true;
		switch (anim->curves[i].applies_to) {
		case anim_element_translation: model->anim_inst.node_transforms[node].translation = anim_curve_sample_f3(&anim->curves[i], &model->anim_inst.curve_last_keyframe[i], time); break;
		case anim_element_scale:       model->anim_inst.node_transforms[node].scale       = anim_curve_sample_f3(&anim->curves[i], &model->anim_inst.curve_last_keyframe[i], time); break;
		case anim_element_rotation:    model->anim_inst.node_transforms[node].rotation    = anim_curve_sample_f4(&anim->curves[i], &model->anim_inst.curve_last_keyframe[i], time); break;
		}
	}

	anim_update_transforms(model, model_node_get_root(model), false);
}

} // namespace sk
