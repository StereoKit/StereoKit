#include "demo_spatial.h"

#include <stereokit.h>
#include <stereokit_ui.h>

using namespace sk;

#include <stdio.h>
#include <map>

struct demo_mesh_t {
	mesh_t mesh;
	pose_t origin;
};
static std::map<spatial_entity_t, demo_mesh_t> demo_meshes;
static material_t demo_mesh_mat = nullptr;
static bool32_t   show_meshes   = true;
static bool32_t   persist_new   = false;

struct cap_name_t { spatial_capability_ cap; const char* name; };
static const cap_name_t cap_names[] = {
	{ spatial_capability_anchor,         "Anchors"        },
	{ spatial_capability_plane_tracking, "Plane Tracking" },
	{ spatial_capability_qr_code,        "QR Codes"       },
	{ spatial_capability_micro_qr,       "Micro QR Codes" },
	{ spatial_capability_aruco,          "ArUco Markers"  },
	{ spatial_capability_april_tag,      "AprilTags"      },
};
static const int32_t cap_name_count = sizeof(cap_names) / sizeof(cap_names[0]);

///////////////////////////////////////////

static const char* label_name(plane_label_ label) {
	switch (label) {
	case plane_label_floor:   return "Floor";
	case plane_label_wall:    return "Wall";
	case plane_label_ceiling: return "Ceiling";
	case plane_label_table:   return "Table";
	default:                  return "Surface";
	}
}

static color32 label_color(plane_label_ label) {
	switch (label) {
	case plane_label_floor:   return {  64, 255,  64, 255 };
	case plane_label_wall:    return {  64,  64, 255, 255 };
	case plane_label_ceiling: return { 255,  64,  64, 255 };
	case plane_label_table:   return { 255, 255,  64, 255 };
	default:                  return { 255, 255, 255, 255 };
	}
}

static void draw_rect(pose_t pose, vec2 size, color32 col) {
	vec3 right = pose.orientation * vec3{ size.x * 0.5f, 0, 0 };
	vec3 up    = pose.orientation * vec3{ 0, size.y * 0.5f, 0 };
	vec3 c[4]  = {
		pose.position - right - up, pose.position + right - up,
		pose.position + right + up, pose.position - right + up };
	for (int32_t i = 0; i < 4; i++)
		line_add(c[i], c[(i + 1) % 4], col, col, 0.005f);
}

static void draw_box(pose_t pose, vec3 size, color32 col) {
	vec3 h = size * 0.5f;
	vec3 c[8];
	for (int32_t i = 0; i < 8; i++)
		c[i] = pose.position + pose.orientation * vec3{ i & 1 ? h.x : -h.x, i & 2 ? h.y : -h.y, i & 4 ? h.z : -h.z };
	const int32_t edges[12][2] = { {0,1},{1,3},{3,2},{2,0}, {4,5},{5,7},{7,6},{6,4}, {0,4},{1,5},{2,6},{3,7} };
	for (int32_t i = 0; i < 12; i++)
		line_add(c[edges[i][0]], c[edges[i][1]], col, col, 0.005f);
}

///////////////////////////////////////////

void demo_spatial_init() {
	demo_mesh_mat = material_copy_id(default_id_material);
	material_set_wireframe(demo_mesh_mat, true);

	spatial_enable(spatial_capabilities());
}

///////////////////////////////////////////

void demo_spatial_update() {
	// Settings window: per-capability toggles and some status
	static pose_t window_pose = pose_t{ {0.4f,0.0f,-0.4f}, quat_lookat({}, {-1,0,1}) };
	ui_window_begin("Spatial Entities", &window_pose);

	spatial_capability_ supported = spatial_capabilities();
	spatial_capability_ enabled   = spatial_get_enabled();
	for (int32_t i = 0; i < cap_name_count; i++) {
		bool32_t is_supported = (supported & cap_names[i].cap) != 0;
		bool32_t on           = is_supported && (enabled & cap_names[i].cap) != 0;
		ui_push_enabled(is_supported);
		if (ui_toggle(cap_names[i].name, on)) {
			if (on) spatial_enable (cap_names[i].cap);
			else    spatial_disable(cap_names[i].cap);
		}
		ui_pop_enabled();
	}

	ui_hseparator();
	char txt[64];
	snprintf(txt, sizeof(txt), "%d entities", spatial_entity_get_count(spatial_component_none));
	ui_label(txt);
	ui_toggle("Show Meshes", show_meshes);

	// Anchor creation + persistence, when the capability is up
	ui_hseparator();
	ui_push_enabled((spatial_get_active() & spatial_capability_anchor) != 0);
	if (ui_button("Create Anchor")) {
		pose_t head = input_head();
		vec3   at   = head.position + head.orientation * vec3_forward * 0.5f;
		spatial_entity_t anchor = spatial_entity_create_anchor({ at, quat_lookat(at, head.position) });
		if (anchor && persist_new)
			spatial_entity_persist(anchor);
	}
	ui_sameline();
	ui_push_enabled(spatial_persistence_available());
	ui_toggle("Persist", persist_new);
	ui_pop_enabled();
	ui_pop_enabled();
	ui_window_end();

	// Draw every entity the registry knows about
	int32_t count = spatial_entity_get_count(spatial_component_none);
	for (int32_t i = 0; i < count; i++) {
		spatial_entity_t entity  = spatial_entity_get_index(spatial_component_none, i);
		bool32_t         tracked = (spatial_entity_get_tracked(entity) & button_state_active) != 0;

		plane_align_ align = plane_align_none;
		plane_label_ label = plane_label_none;
		spatial_entity_get_plane(entity, &align, &label);
		color32 col = tracked ? label_color(label) : color32{ 128,128,128,255 };

		pose_t center = {};
		vec2   size2;
		vec3   size3;
		bool32_t has_rect = spatial_entity_get_bounds2d(entity, &center, &size2);
		if (has_rect) {
			draw_rect(center, size2, col);
			line_add_axis(center, 0.05f);
		}
		if (spatial_entity_get_bounds3d(entity, &center, &size3))
			draw_box(center, size3, col);

		pose_t      poly_origin;
		const vec2* poly_verts;
		int32_t     poly_count;
		if (spatial_entity_get_polygon(entity, &poly_origin, &poly_verts, &poly_count) && poly_count > 1) {
			for (int32_t v = 0; v < poly_count; v++) {
				vec3 a = poly_origin.position + poly_origin.orientation * vec3{ poly_verts[v].x,                    poly_verts[v].y,                    0 };
				vec3 b = poly_origin.position + poly_origin.orientation * vec3{ poly_verts[(v+1)%poly_count].x, poly_verts[(v+1)%poly_count].y, 0 };
				line_add(a, b, col, col, 0.003f);
			}
		}

		// Anchors draw as an axis gizmo, with their persist id when stored
		pose_t anchor_pose;
		if (spatial_entity_get_anchor(entity, &anchor_pose)) {
			line_add_axis(anchor_pose, 0.1f);

			uint8_t uuid[16];
			char    anchor_txt[16];
			if (spatial_entity_get_persist_id(entity, uuid))
				snprintf(anchor_txt, sizeof(anchor_txt), "%02x%02x%02x%02x", uuid[0], uuid[1], uuid[2], uuid[3]);
			else
				snprintf(anchor_txt, sizeof(anchor_txt), "anchor");
			text_add_at(anchor_txt, pose_matrix(anchor_pose), 0, pivot_top_center);
		}

		// A text label: decoded marker data, marker id, or plane label
		marker_type_ marker_type;
		uint32_t     marker_id;
		char         name[128] = {};
		if (spatial_entity_get_marker(entity, &marker_type, &marker_id)) {
			const char* marker_text = spatial_entity_get_marker_text(entity);
			if (marker_text) snprintf(name, sizeof(name), "%s",         marker_text);
			else             snprintf(name, sizeof(name), "Marker #%u", marker_id);
		} else if (label != plane_label_none) {
			snprintf(name, sizeof(name), "%s", label_name(label));
		}
		if (name[0] != '\0' && has_rect)
			text_add_at(name, pose_matrix(center), 0, pivot_center);

		// Wireframe entity meshes, refilled only when the data changes
		if (show_meshes && (spatial_entity_get_components(entity) & spatial_component_mesh)) {
			auto it = demo_meshes.find(entity);
			if (it == demo_meshes.end()) {
				demo_mesh_t dm = { mesh_create() };
				if (spatial_entity_get_mesh(entity, dm.mesh, &dm.origin)) it = demo_meshes.insert({ entity, dm }).first;
				else                                                      mesh_release(dm.mesh);
			} else {
				mesh_t refill = (spatial_entity_get_changed(entity) & spatial_component_mesh) ? it->second.mesh : nullptr;
				spatial_entity_get_mesh(entity, refill, &it->second.origin);
			}
			if (it != demo_meshes.end())
				render_add_mesh(it->second.mesh, demo_mesh_mat, pose_matrix(it->second.origin));
		}
	}
}

///////////////////////////////////////////

void demo_spatial_shutdown() {
	for (auto& kv : demo_meshes)
		mesh_release(kv.second.mesh);
	demo_meshes.clear();
	material_release(demo_mesh_mat);
	demo_mesh_mat = nullptr;

	spatial_disable(spatial_capabilities());
}
