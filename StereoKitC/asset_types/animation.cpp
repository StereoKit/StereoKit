#include "animation.h"
#include "model.h"
#include "../sk_math.h"

namespace sk {

array_t<model_t> animation_list = {};

///////////////////////////////////////////

int32_t anim_frame(const anim_curve_t *curve, int32_t prev_index, float t) {
	if (prev_index >= 0) {
		if (                                          curve->keyframe_times[prev_index  ] <= t && curve->keyframe_times[prev_index+1] > t) return prev_index;
		if (prev_index + 2 < curve->keyframe_count && curve->keyframe_times[prev_index+1] <= t && curve->keyframe_times[prev_index+2] > t) return prev_index + 1;
	}
	if (t < curve->keyframe_times[0] || curve->keyframe_count == 1) return 0;
	if (t > curve->keyframe_times[curve->keyframe_count - 1]) return curve->keyframe_count - 1;

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
	float   pct   = math_saturate((t - curve->keyframe_times[frame]) / (curve->keyframe_times[frame + 1] - curve->keyframe_times[frame]));
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
	float   pct   = math_saturate((t - curve->keyframe_times[frame]) / (curve->keyframe_times[frame + 1] - curve->keyframe_times[frame]));
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

static void anim_update_transforms(model_t model, model_node_id node_id, bool dirty_world) {
	while (node_id != -1) {
		model_node_t     *node  = &model->nodes[node_id];
		anim_transform_t *tr    = &model->anim_inst.node_transforms[node_id];
		bool              dirty = dirty_world;

		// Only update this node_id if the animation touched the transform
		if (tr->dirty) {
			node->transform_local = matrix_trs(tr->translation, tr->rotation, tr->scale);
			tr->dirty             = false;
			dirty                 = true;
		}

		// If this node_id or a parent node_id was touched, we need to update the world
		// transform.
		if (dirty == true) {
			if (node->parent >= 0) node->transform_model = node->transform_local * model->nodes[node->parent].transform_model;
			else                   node->transform_model = node->transform_local;

			if (node->visual >= 0)
				model->visuals[node->visual].transform_model = node->transform_model;
		}

		anim_update_transforms(model, node->child, dirty);
		node_id = node->sibling;
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
