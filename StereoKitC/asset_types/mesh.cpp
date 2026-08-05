#include "../stereokit.h"
#include "../sk_memory.h"
#include "../sk_math.h"
#include "../sk_math_dx.h"
#include "mesh.h"
#include "assets.h"
#include "../systems/render.h"
#include "../systems/vert_format.h"

#include <stdio.h>
#include <string.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>

using namespace DirectX;

namespace sk {

void mesh_update_label(mesh_t mesh);
// Calculates bounds via the format's position component, which must be a
// float3 for this to succeed.
static bool mesh_calculate_bounds(int32_t format_id, const void* verts, int32_t vert_count, bounds_t* out_bounds);

///////////////////////////////////////////

// Unaligned safe vec3 load/store, packed vertex formats can put float
// components at any byte offset. The memcpy compiles to plain loads.
static inline XMVECTOR xm_load_v3 (const void* at) {
	XMFLOAT3 v;
	memcpy(&v, at, sizeof(v));
	return XMLoadFloat3(&v);
}
static inline void     xm_store_v3(void* at, XMVECTOR value) {
	XMFLOAT3 v;
	XMStoreFloat3(&v, value);
	memcpy(at, &v, sizeof(v));
}

///////////////////////////////////////////

void mesh_set_keep_data(mesh_t mesh, bool32_t keep_data) {
	if (mesh_has_skin(mesh) && !keep_data) {
		log_warn("Skinned meshes must keep their data, ignoring mesh_set_keep_data call.");
		return;
	}

	mesh->discard_data = !keep_data;
	if (mesh->discard_data) {
		sk_free(mesh->verts);
		sk_free(mesh->inds );
	}
}

///////////////////////////////////////////

bool32_t mesh_get_keep_data(mesh_t mesh) {
	return !mesh->discard_data;
}

///////////////////////////////////////////

// Shared vertex upload path for both vert_t and custom format meshes. The
// mesh manages its own reference to format_id here, callers keep theirs.
static void _mesh_set_verts(mesh_t mesh, const void *vertices, uint32_t vertex_count, int32_t format_id, bool32_t calculate_bounds, bool update_original) {
	if (mesh->header.state >= asset_state_loaded && mesh->vert_count == 0) {
		log_diagf("mesh_set_verts: Setting verts after mesh is already loaded. For best results, set verts before inds. (%s)", mesh->header.id_text);
	}

	uint32_t stride = vert_format_get_stride(format_id);

	// Swap the mesh's format reference if it changed. Skinned meshes can't
	// change format, their deformed copy and weights pair with the current
	// format's stride and vertex order.
	if (format_id != mesh->vert_format) {
		if (mesh_has_skin(mesh)) {
			log_err("mesh_set_verts: can't change the vertex format of a mesh with skinning data");
			return;
		}
		vert_format_addref (format_id);
		vert_format_release(mesh->vert_format);
		mesh->vert_format = format_id;
	}
	mesh->vert_stride        = stride;
	mesh->gpu_mesh.vert_type = vert_format_get_skr(format_id);

	// Keep track of vertex data for use on CPU side
	if (!mesh->discard_data && update_original) {
		uint32_t bytes = vertex_count * stride;
		if (mesh->vert_capacity_bytes < bytes) {
			mesh->verts               = (vert_t*)sk_realloc(mesh->verts, bytes);
			mesh->vert_capacity_bytes = bytes;
		}
		memcpy(mesh->verts, vertices, bytes);
	}

	// skr_mesh_set_verts handles static-to-dynamic conversion and resizing internally
	bool first_set = !skr_mesh_is_valid(&mesh->gpu_mesh);
	if (skr_mesh_set_verts(&mesh->gpu_mesh, vertices, vertex_count) != skr_err_success) {
		log_err("mesh_set_verts: Failed to set vertex data");
	}
	if (first_set) {
		mesh_update_label(mesh);
	}

	mesh->vert_count = vertex_count;

	if (calculate_bounds && vertex_count > 0) {
		if (!mesh_calculate_bounds(format_id, vertices, vertex_count, &mesh->bounds))
			log_warnf("mesh_set_verts: can't calculate bounds without a float3 position component (%s)", mesh->header.id_text);
	}
}

///////////////////////////////////////////

void mesh_set_verts(mesh_t mesh, const vert_t *vertices, int32_t vertex_count, bool32_t calculate_bounds) {
	struct vert_upload_job_t {
		mesh_t        mesh;
		const vert_t *vertices;
		int32_t       vertex_count;
		bool32_t      calculate_bounds;
	};
	vert_upload_job_t job_data = {mesh, vertices, vertex_count, calculate_bounds};

	assets_execute_blocking([](void *data) {
		vert_upload_job_t *job_data = (vert_upload_job_t *)data;
		_mesh_set_verts(job_data->mesh, job_data->vertices, job_data->vertex_count, VERT_FORMAT_DEFAULT, job_data->calculate_bounds, true);

		return (bool32_t)true;
	}, &job_data);
}

///////////////////////////////////////////

void mesh_set_verts_fmt(mesh_t mesh, const vert_component_t *format, int32_t component_count, const void *vertex_data, int32_t vertex_count, bool32_t calculate_bounds) {
	int32_t format_id = vert_format_ref(format, component_count);
	if (format_id < 0) return;
	if (format_id != mesh->vert_format && mesh_has_skin(mesh)) {
		log_err("mesh_set_verts_fmt: can't change the vertex format of a mesh with skinning data");
		vert_format_release(format_id);
		return;
	}

	struct vert_fmt_upload_job_t {
		mesh_t      mesh;
		const void *vertices;
		int32_t     vertex_count;
		int32_t     format_id;
		bool32_t    calculate_bounds;
	};
	vert_fmt_upload_job_t job_data = {mesh, vertex_data, vertex_count, format_id, calculate_bounds};

	assets_execute_blocking([](void *data) {
		vert_fmt_upload_job_t *job_data = (vert_fmt_upload_job_t *)data;
		_mesh_set_verts(job_data->mesh, job_data->vertices, job_data->vertex_count, job_data->format_id, job_data->calculate_bounds, true);

		return (bool32_t)true;
	}, &job_data);

	vert_format_release(format_id);
}

///////////////////////////////////////////

void mesh_get_verts(mesh_t mesh, vert_t *&out_vertices, int32_t &out_vertex_count, memory_ reference_mode) {
	out_vertex_count = 0;
	out_vertices     = nullptr;

	if (mesh->vert_format != VERT_FORMAT_DEFAULT) {
		log_warn("mesh_get_verts: this mesh uses a custom vertex format, use mesh_get_verts_fmt instead");
		return;
	}
	out_vertex_count = mesh->verts == nullptr ? 0 : mesh->vert_count;

	if (reference_mode == memory_copy && mesh->verts != nullptr && mesh->vert_count > 0) {
		out_vertices = sk_malloc_t(vert_t, mesh->vert_count);
		memcpy(out_vertices, mesh->verts, sizeof(vert_t) * mesh->vert_count);
	} else if (reference_mode == memory_reference) {
		out_vertices = mesh->verts;
	}
}

///////////////////////////////////////////

void mesh_get_verts_fmt(mesh_t mesh, vert_component_t **out_format, int32_t *out_component_count, void **out_vertex_data, int32_t *out_vertex_count, memory_ reference_mode) {
	*out_format          = nullptr;
	*out_component_count = 0;
	*out_vertex_data     = nullptr;
	*out_vertex_count    = mesh->verts == nullptr ? 0 : mesh->vert_count;

	int32_t                 component_count = 0;
	const vert_component_t* components      = vert_format_get_components(mesh->vert_format, &component_count);
	*out_component_count = component_count;

	if (reference_mode == memory_copy) {
		*out_format = sk_malloc_t(vert_component_t, component_count);
		memcpy(*out_format, components, sizeof(vert_component_t) * component_count);
		if (mesh->verts != nullptr && mesh->vert_count > 0) {
			uint32_t bytes = mesh->vert_count * mesh->vert_stride;
			*out_vertex_data = sk_malloc(bytes);
			memcpy(*out_vertex_data, mesh->verts, bytes);
		}
	} else if (reference_mode == memory_reference) {
		*out_format      = (vert_component_t*)components;
		*out_vertex_data = mesh->verts;
	}
}

///////////////////////////////////////////

int32_t mesh_get_vert_count(mesh_t mesh) {
	return mesh->vert_count;
}

///////////////////////////////////////////

void _mesh_set_inds (mesh_t mesh, const vind_t *indices, uint32_t index_count) {
	if (index_count % 3 != 0) {
		log_err("mesh_set_inds index_count must be a multiple of 3!");
		return;
	}

	// Keep track of index data for use on CPU side
	if (!mesh->discard_data) {
		if (mesh->ind_capacity < index_count) {
			mesh->inds         = sk_realloc_t(vind_t, mesh->inds, index_count);
			mesh->ind_capacity = index_count;
		}
		memcpy(mesh->inds, indices, sizeof(vind_t) * index_count);
	}

	// skr_mesh_set_inds handles static-to-dynamic conversion and resizing internally
	bool first_set = mesh->ind_count == 0;
	if (skr_mesh_set_inds(&mesh->gpu_mesh, indices, index_count) != skr_err_success) {
		log_err("mesh_set_inds: Failed to set index data");
	}
	if (first_set) {
		mesh_update_label(mesh);
	}

	mesh->ind_count    = index_count;
	mesh->ind_draw     = index_count;
	mesh->header.state = asset_state_loaded;
}

///////////////////////////////////////////

void mesh_set_inds(mesh_t mesh, const vind_t *indices,  int32_t index_count) {
	struct ind_upload_job_t {
		mesh_t        mesh;
		const vind_t *indices;
		int32_t       index_count;
	};
	ind_upload_job_t job_data = {mesh, indices, index_count};

	assets_execute_blocking([](void *data) {
		ind_upload_job_t *job_data = (ind_upload_job_t *)data;
		_mesh_set_inds(job_data->mesh, job_data->indices, job_data->index_count);
		
		return (bool32_t)true;
	}, &job_data);
}

///////////////////////////////////////////

///////////////////////////////////////////
// Async mesh loading infrastructure     //
///////////////////////////////////////////

struct mesh_load_t {
	void*    verts;
	vind_t*  inds;
	int32_t  vert_count;
	int32_t  ind_count;
	int32_t  vert_format; // load holds its own registry ref
	bool32_t calc_bounds;
};

static bool32_t mesh_load_process(asset_task_t*, asset_header_t* asset, void *data) {
	mesh_t       mesh = (mesh_t)asset;
	mesh_load_t* load = (mesh_load_t*)data;

	if (load->calc_bounds && load->vert_count > 0) {
		if (!mesh_calculate_bounds(load->vert_format, load->verts, load->vert_count, &mesh->bounds))
			log_warnf("mesh_set_data: can't calculate bounds without a float3 position component (%s)", mesh->header.id_text);
	}

	mesh->header.state = asset_state_loaded_meta;
	return true;
}

static bool32_t mesh_load_upload(asset_task_t*, asset_header_t* asset, void *data) {
	mesh_t       mesh = (mesh_t)asset;
	mesh_load_t* load = (mesh_load_t*)data;

	// Gate the renderer before swapping gpu buffers. When re-uploading an
	// already-loaded mesh, _mesh_set_verts replaces the vertex buffer
	// before _mesh_set_inds gets a chance to update ind_draw, so a main
	// thread draw between those two steps would pair new verts with the
	// old ind_draw/ind buffer. Zeroing ind_draw here keeps the renderer
	// skipping this mesh until _mesh_set_inds restores it. Old gpu
	// buffers + old ind_draw are mutually consistent up to this point.
	mesh->ind_draw = 0;

	// Upload from load's data, then hand ownership to the mesh or
	// discard. The load task exclusively owns verts/inds until this
	// point, so there's no race with the main thread for those.
	if (load->vert_count > 0) _mesh_set_verts(mesh, load->verts, load->vert_count, load->vert_format, false, false);
	if (load->ind_count  > 0) _mesh_set_inds     (mesh, load->inds,  load->ind_count);

	if (!mesh->discard_data) {
		mesh->verts               = (vert_t*)load->verts;
		mesh->vert_capacity_bytes = load->vert_count * vert_format_get_stride(load->vert_format);
		mesh->inds                = load->inds;
		mesh->ind_capacity        = load->ind_count;

		load->verts = nullptr;
		load->inds  = nullptr;
	}

	return true;
}

static void mesh_load_free(asset_header_t*, void *data) {
	mesh_load_t* load = (mesh_load_t*)data;
	vert_format_release(load->vert_format);
	sk_free(load->verts);
	sk_free(load->inds);
	sk_free(load);
}

static void mesh_load_on_failure(asset_header_t* asset, void *) {
	((mesh_t)asset)->header.state = asset_state_error;
}

///////////////////////////////////////////

// Shared implementation for default and custom format data uploads, the
// caller keeps its own reference to format_id.
static void _mesh_set_data(mesh_t mesh, int32_t format_id, const void* vertices, int32_t vertex_count, const vind_t* indices, int32_t index_count, mesh_data_ flags, int32_t priority) {
	bool32_t calc_bounds = (flags & mesh_data_calc_bounds) != 0;

	if (!(flags & mesh_data_async)) {
		struct mesh_upload_job_t {
			mesh_t        mesh;
			const void*   vertices;
			int32_t       vertex_count;
			const vind_t* indices;
			int32_t       index_count;
			int32_t       format_id;
			bool32_t      calc_bounds;
		};
		mesh_upload_job_t job_data = {mesh, vertices, vertex_count, indices, index_count, format_id, calc_bounds};

		assets_execute_blocking([](void *data) {
			mesh_upload_job_t *job_data = (mesh_upload_job_t *)data;
			if (job_data->vertex_count > 0) _mesh_set_verts(job_data->mesh, job_data->vertices, job_data->vertex_count, job_data->format_id, job_data->calc_bounds, true);
			if (job_data->index_count  > 0) _mesh_set_inds     (job_data->mesh, job_data->indices,  job_data->index_count);
			return (bool32_t)true;
		}, &job_data);
	} else {
		mesh->header.state = asset_state_loading;
		mesh->vert_count   = vertex_count;
		mesh->ind_count    = index_count;

		uint32_t stride = vert_format_get_stride(format_id);

		// The load's format reference protects against the mesh's format
		// being swapped from another thread mid-task.
		vert_format_addref(format_id);

		// The load task exclusively owns the vert/ind data until
		// upload completes. mesh->verts/inds stay null during
		// loading — the upload task hands ownership to the mesh
		// afterward if discard_data is false.
		mesh_load_t *load_data = sk_malloc_zero_t(mesh_load_t, 1);
		load_data->calc_bounds = calc_bounds;
		load_data->vert_count  = vertex_count;
		load_data->ind_count   = index_count;
		load_data->vert_format = format_id;
		if (vertex_count > 0) {
			load_data->verts = sk_malloc(vertex_count * stride);
			memcpy(load_data->verts, vertices, vertex_count * stride);
		}
		if (index_count > 0) {
			load_data->inds = sk_malloc_t(vind_t, index_count);
			memcpy(load_data->inds, indices, sizeof(vind_t) * index_count);
		}

		static const asset_load_action_t actions[] = {
			mesh_load_process,
			mesh_load_upload,
		};

		asset_task_t task = {};
		task.asset        = &mesh->header;
		task.load_data    = load_data;
		task.actions      = (asset_load_action_t *)actions;
		task.action_count = _countof(actions);
		task.free_data    = mesh_load_free;
		task.on_failure   = mesh_load_on_failure;
		task.priority     = priority;
		task.sort         = asset_sort(priority, asset_complexity_bytes(vertex_count * stride + index_count * sizeof(vind_t)));

		assets_add_task(task);
	}
}

///////////////////////////////////////////

void mesh_set_data(mesh_t mesh, const vert_t* vertices, int32_t vertex_count, const vind_t* indices, int32_t index_count, mesh_data_ flags, int32_t priority) {
	_mesh_set_data(mesh, VERT_FORMAT_DEFAULT, vertices, vertex_count, indices, index_count, flags, priority);
}

///////////////////////////////////////////

void mesh_set_data_fmt(mesh_t mesh, const vert_component_t* format, int32_t component_count, const void* vertex_data, int32_t vertex_count, const vind_t* indices, int32_t index_count, mesh_data_ flags, int32_t priority) {
	int32_t format_id = vert_format_ref(format, component_count);
	if (format_id < 0) return;
	if (format_id != mesh->vert_format && mesh_has_skin(mesh)) {
		log_err("mesh_set_data_fmt: can't change the vertex format of a mesh with skinning data");
		vert_format_release(format_id);
		return;
	}

	_mesh_set_data(mesh, format_id, vertex_data, vertex_count, indices, index_count, flags, priority);

	vert_format_release(format_id);
}

///////////////////////////////////////////

void mesh_get_inds(mesh_t mesh, vind_t *&out_indices, int32_t &out_index_count, memory_ reference_mode) {
	out_index_count = mesh->inds == nullptr ? 0 : (int32_t)mesh->ind_count;
	out_indices     = nullptr;
	
	if (reference_mode == memory_copy && mesh->inds != nullptr && mesh->ind_count > 0) {
		out_indices = sk_malloc_t(vind_t, mesh->ind_count);
		memcpy(out_indices, mesh->inds, sizeof(vind_t) * mesh->ind_count);
	} else if (reference_mode == memory_reference) {
		out_indices = mesh->inds;
	}
}

///////////////////////////////////////////

int32_t mesh_get_ind_count(mesh_t mesh) {
	return (int32_t)mesh->ind_count;
}

///////////////////////////////////////////

bool mesh_calculate_normals(int32_t format_id, void *verts, int32_t vert_count, const vind_t *inds, int32_t ind_count) {
	// This needs a position to read, and a normal to write back to.
	vert_fmt_ pos_fmt  = vert_fmt_none, norm_fmt  = vert_fmt_none;
	int32_t   pos_ct   = 0,             norm_ct   = 0;
	int32_t   pos_off  = vert_format_semantic_offset(format_id, vert_semantic_position, 0, &pos_fmt,  &pos_ct);
	int32_t   norm_off = vert_format_semantic_offset(format_id, vert_semantic_normal,   0, &norm_fmt, &norm_ct);
	if (pos_off < 0 || norm_off < 0)
		return false;

	uint32_t stride = vert_format_get_stride(format_id);
	uint8_t* data   = (uint8_t*)verts;

	// Float positions read strided straight from the vertex data, other
	// position formats decode into a dense scratch array first.
	const uint8_t* pos_base   = data + pos_off;
	uint32_t       pos_stride = stride;
	XMFLOAT4*      scratch    = nullptr;
	if (!(pos_fmt == vert_fmt_f32 && pos_ct >= 3)) {
		scratch = sk_malloc_t(XMFLOAT4, vert_count);
		for (int32_t i = 0; i < vert_count; i++) {
			vec4 p = {};
			vert_format_decode(format_id, data + i*stride, vert_semantic_position, 0, &p);
			memcpy(&scratch[i], &p, sizeof(p));
		}
		pos_base   = (uint8_t*)scratch;
		pos_stride = sizeof(XMFLOAT4);
	}

	// Normals accumulate in a dense 16 byte array, partial width stores
	// into the vertices themselves would cripple the SIMD here.
	XMFLOAT4* acc = sk_malloc_zero_t(XMFLOAT4, vert_count);
	for (int32_t i = 0; i < ind_count; i+=3) {
		vind_t   i1 = inds[i  ];
		vind_t   i2 = inds[i+1];
		vind_t   i3 = inds[i+2];
		XMVECTOR p2 = xm_load_v3(pos_base + i2*pos_stride);
		// Unnormalized cross product length is twice the triangle's area,
		// which gives us area weighted normals for free!
		XMVECTOR n  = XMVector3Cross(
			XMVectorSubtract(xm_load_v3(pos_base + i3*pos_stride), p2),
			XMVectorSubtract(xm_load_v3(pos_base + i1*pos_stride), p2));
		XMStoreFloat4(&acc[i1], XMVectorAdd(XMLoadFloat4(&acc[i1]), n));
		XMStoreFloat4(&acc[i2], XMVectorAdd(XMLoadFloat4(&acc[i2]), n));
		XMStoreFloat4(&acc[i3], XMVectorAdd(XMLoadFloat4(&acc[i3]), n));
	}

	bool norm_direct = norm_fmt == vert_fmt_f32 && norm_ct == 3;
	for (int32_t i = 0; i < vert_count; i++) {
		XMFLOAT4 n;
		XMStoreFloat4(&n, XMVector3Normalize(XMLoadFloat4(&acc[i])));
		if (norm_direct) memcpy(data + i*stride + norm_off, &n, sizeof(vec3));
		else             vert_format_encode(format_id, data + i*stride, vert_semantic_normal, 0, vec4{n.x, n.y, n.z, 0});
	}

	sk_free(scratch);
	sk_free(acc);
	return true;
}

///////////////////////////////////////////

static bounds_t mesh_calculate_bounds_strided(const void* verts, int32_t vert_count, int32_t stride, int32_t position_offset) {
	// Calculate the bounds for this mesh by searching it for min and max
	// values! This uses DirectXMath's SIMD capabilities, and uses multiple
	// separate accumulators to reduce operation dependencies.
	const uint8_t* pos = (const uint8_t*)verts + position_offset;

	XMVECTOR min_a = xm_load_v3(pos);
	XMVECTOR min_b = min_a;
	XMVECTOR min_c = min_a;
	XMVECTOR min_d = min_a;
	XMVECTOR max_a = min_a;
	XMVECTOR max_b = min_a;
	XMVECTOR max_c = min_a;
	XMVECTOR max_d = min_a;

	const uint8_t* curr = pos;
	for (int32_t i = 0; i < vert_count/4; i++) {
		XMVECTOR pt_a = xm_load_v3(curr           );
		min_a = XMVectorMin(min_a, pt_a);
		max_a = XMVectorMax(max_a, pt_a);
		XMVECTOR pt_b = xm_load_v3(curr + stride  );
		min_b = XMVectorMin(min_b, pt_b);
		max_b = XMVectorMax(max_b, pt_b);
		XMVECTOR pt_c = xm_load_v3(curr + stride*2);
		min_c = XMVectorMin(min_c, pt_c);
		max_c = XMVectorMax(max_c, pt_c);
		XMVECTOR pt_d = xm_load_v3(curr + stride*3);
		min_d = XMVectorMin(min_d, pt_d);
		max_d = XMVectorMax(max_d, pt_d);
		curr += stride*4;
	}
	for (int32_t i = (vert_count / 4) * 4; i < vert_count; i++) {
		XMVECTOR pt_a = xm_load_v3(curr);
		min_a = XMVectorMin(min_a, pt_a);
		max_a = XMVectorMax(max_a, pt_a);
		curr += stride;
	}

	XMVECTOR min = XMVectorMin(min_a, min_b);
	min = XMVectorMin(min, min_c);
	min = XMVectorMin(min, min_d);
	XMVECTOR max = XMVectorMax(max_a, max_b);
	max = XMVectorMax(max, max_c);
	max = XMVectorMax(max, max_d);

	XMVECTOR center     = XMVectorMultiplyAdd(min, g_XMOneHalf, XMVectorMultiply(max, g_XMOneHalf));
	XMVECTOR dimensions = XMVectorSubtract(max, min);
	bounds_t bounds     = {};
	XMStoreFloat3((XMFLOAT3*)&bounds.center,     center);
	XMStoreFloat3((XMFLOAT3*)&bounds.dimensions, dimensions);

	return bounds;
}

///////////////////////////////////////////

static bool mesh_calculate_bounds(int32_t format_id, const void* verts, int32_t vert_count, bounds_t* out_bounds) {
	vert_fmt_ pos_fmt   = vert_fmt_none;
	int32_t   pos_count = 0;
	int32_t   pos_off   = vert_format_semantic_offset(format_id, vert_semantic_position, 0, &pos_fmt, &pos_count);
	if (pos_off < 0 || pos_fmt != vert_fmt_f32 || pos_count < 3)
		return false;

	*out_bounds = mesh_calculate_bounds_strided(verts, vert_count, vert_format_get_stride(format_id), pos_off);
	return true;
}

///////////////////////////////////////////

void mesh_set_draw_inds(mesh_t mesh, int32_t index_count) {
	uint32_t u_count = index_count;
	if (u_count > mesh->ind_count) {
		u_count = mesh->ind_count;
		log_warn("mesh_set_draw_inds: Can't render more indices than the mesh has! Capping...");
	}
	mesh->ind_draw = u_count;
}

///////////////////////////////////////////

void mesh_set_bounds(mesh_t mesh, const bounds_t &bounds) {
	mesh->bounds = bounds;
}

///////////////////////////////////////////

bounds_t mesh_get_bounds(mesh_t mesh) {
	return mesh->bounds;
}

bool32_t mesh_has_skin(mesh_t mesh) {
	return mesh->skin_data.bone_data != nullptr;
}

///////////////////////////////////////////

void _mesh_set_weights(mesh_t mesh, const uint16_t* bone_ids_4, int32_t bone_id_4_count, const vec4* bone_weights, int32_t bone_weight_count) {
	for (int32_t i = 0; i < bone_weight_count; i++) {
		// Convert the weights to 8-bit integers, for a more memory efficient
		// representation. This will also ensure the weights are normalized.
		float   multiplier = 255.0f / (bone_weights[i].x + bone_weights[i].y + bone_weights[i].z + bone_weights[i].w);
		uint8_t weight[4]  = {
			(uint8_t)fminf(255, roundf(bone_weights[i].x * multiplier)),
			(uint8_t)fminf(255, roundf(bone_weights[i].y * multiplier)),
			(uint8_t)fminf(255, roundf(bone_weights[i].z * multiplier)),
			(uint8_t)fminf(255, roundf(bone_weights[i].w * multiplier)) 
		};

		// A small percentage of the time, the weights will add up to 254 or
		// 256, which will cause artifacts when skinning. This code certifies
		// that the weights always add up to 255.
		int32_t sum = weight[0] + weight[1] + weight[2] + weight[3];
		if      (sum < 255) weight[0] += (uint8_t)(255 - sum);
		else if (sum > 255) {
			// Remove weights from the last item first. This loop will almost
			// certainly execute only once.
			while (sum > 255) {
				if      (weight[3] > 0) { weight[3]--; sum--; }
				else if (weight[2] > 0) { weight[2]--; sum--; }
				else if (weight[1] > 0) { weight[1]--; sum--; }
				else if (weight[0] > 0) { weight[0]--; sum--; }
			}
		}

		mesh->skin_data.bone_data[i].weight[0] = weight[0];
		mesh->skin_data.bone_data[i].weight[1] = weight[1];
		mesh->skin_data.bone_data[i].weight[2] = weight[2];
		mesh->skin_data.bone_data[i].weight[3] = weight[3];
		// Zero weight slots still get their palette entry read by the
		// branchless blend in mesh_update_skin, keep their ids valid.
		mesh->skin_data.bone_data[i].bone_id[0] = weight[0] > 0 ? bone_ids_4[i * 4 + 0] : 0;
		mesh->skin_data.bone_data[i].bone_id[1] = weight[1] > 0 ? bone_ids_4[i * 4 + 1] : 0;
		mesh->skin_data.bone_data[i].bone_id[2] = weight[2] > 0 ? bone_ids_4[i * 4 + 2] : 0;
		mesh->skin_data.bone_data[i].bone_id[3] = weight[3] > 0 ? bone_ids_4[i * 4 + 3] : 0;
	}
}

///////////////////////////////////////////

bool _mesh_set_skin(mesh_t mesh, const bone_weight_t *bone_weights, uint32_t bone_weight_count, int32_t bone_count) {
	if (mesh->discard_data) {
		log_err("mesh_set_skin: can't work with a mesh that doesn't keep data, ensure mesh_get_keep_data() is true");
		return false;
	}
	if (mesh->verts == nullptr) {
		log_err("mesh_set_skin: mesh has no vertex data, ensure mesh data is loaded before setting skin");
		return false;
	}
	if (vert_format_semantic_offset(mesh->vert_format, vert_semantic_position, 0, nullptr, nullptr) < 0) {
		log_err("mesh_set_skin: the mesh's vertex format has no position component");
		return false;
	}

	mesh->skin_data.bone_data      = sk_malloc_t(bone_weight_t, bone_weight_count);
	mesh->skin_data.deformed_verts = sk_malloc(mesh->vert_count * mesh->vert_stride);
	memcpy(mesh->skin_data.deformed_verts, mesh->verts, mesh->vert_count * mesh->vert_stride);
	if (bone_weights != nullptr) {
		memcpy(mesh->skin_data.bone_data, bone_weights, sizeof(bone_weight_t) * bone_weight_count);
		// Zero weight slots still get their palette entry read by the
		// branchless blend in mesh_update_skin, keep their ids valid.
		for (uint32_t i = 0; i < bone_weight_count; i++)
			for (int32_t k = 0; k < 4; k++)
				if (mesh->skin_data.bone_data[i].weight[k] == 0) mesh->skin_data.bone_data[i].bone_id[k] = 0;
	}

	mesh->skin_data.bone_inverse_transforms = sk_malloc_t(matrix, bone_count);
	mesh->skin_data.bone_transforms         = sk_malloc_t(matrix, bone_count);
	memset(mesh->skin_data.bone_inverse_transforms, 0, sizeof(matrix) * bone_count);
	memset(mesh->skin_data.bone_transforms,         0, sizeof(matrix) * bone_count);

	mesh->skin_data.bone_count = bone_count;

	return true;
}

///////////////////////////////////////////

void mesh_set_skin(mesh_t mesh, const uint16_t *bone_ids_4, int32_t bone_id_4_count, const vec4 *bone_weights, int32_t bone_weight_count, const matrix *in_arr_bone_resting_transforms, int32_t bone_count) {
	if (bone_weight_count != bone_id_4_count || bone_weight_count != (int32_t)mesh->vert_count) {
		log_err("mesh_set_skin: bone_weights, bone_ids_4 and vertex counts must match exactly");
		return;
	}

	if (_mesh_set_skin(mesh, nullptr, bone_weight_count, bone_count)) {
		_mesh_set_weights(mesh, bone_ids_4, bone_id_4_count, bone_weights, bone_weight_count);
		for (int32_t i = 0; i < bone_count; i++) {
			mesh->skin_data.bone_inverse_transforms[i] = matrix_invert(in_arr_bone_resting_transforms[i]);
		}
	}
}

///////////////////////////////////////////

void mesh_set_skin_inv(mesh_t mesh, const bone_weight_t* bone_weights, uint32_t bone_weight_count, const matrix *in_arr_bone_resting_transforms_inverted, int32_t bone_count) {
	if (_mesh_set_skin(mesh, bone_weights, bone_weight_count, bone_count)) {
		memcpy(mesh->skin_data.bone_inverse_transforms, in_arr_bone_resting_transforms_inverted, sizeof(matrix) * bone_count);
	}
}

///////////////////////////////////////////

void mesh_update_skin(mesh_t mesh, const matrix *bone_transforms, int32_t bone_count) {
	for (int32_t i = 0; i < bone_count; i++) {
		mesh->skin_data.bone_transforms[i] = mesh->skin_data.bone_inverse_transforms[i] * bone_transforms[i];
	}

	// Positions and normals read and write through the mesh's vertex
	// format, float components directly, anything else via the codec.
	vert_fmt_ pos_fmt  = vert_fmt_none, norm_fmt  = vert_fmt_none;
	int32_t   pos_ct   = 0,             norm_ct   = 0;
	int32_t   pos_off  = vert_format_semantic_offset(mesh->vert_format, vert_semantic_position, 0, &pos_fmt,  &pos_ct);
	int32_t   norm_off = vert_format_semantic_offset(mesh->vert_format, vert_semantic_normal,   0, &norm_fmt, &norm_ct);
	bool      pos_f32  = pos_fmt  == vert_fmt_f32 && pos_ct  >= 3;
	bool      norm_f32 = norm_fmt == vert_fmt_f32 && norm_ct == 3;
	uint32_t  stride   = mesh->vert_stride;
	const uint8_t* rest = (const uint8_t*)mesh->verts;
	uint8_t*       def  = (      uint8_t*)mesh->skin_data.deformed_verts;

	XMFLOAT3 xmmin = {  FLT_MAX,   FLT_MAX,   FLT_MAX };
	XMFLOAT3 xmmax = { -FLT_MAX,  -FLT_MAX,  -FLT_MAX };
	XMVECTOR min   = XMLoadFloat3(&xmmin);
	XMVECTOR max   = XMLoadFloat3(&xmmax);
	for (uint32_t i = 0; i < mesh->vert_count; i++) {
		XMVECTOR pos, norm;
		if (pos_f32) {
			pos = xm_load_v3(rest + i*stride + pos_off);
		} else {
			vec4 p = {};
			vert_format_decode(mesh->vert_format, rest + i*stride, vert_semantic_position, 0, &p);
			pos = XMLoadFloat4((const XMFLOAT4*)&p);
		}
		if (norm_f32) {
			norm = xm_load_v3(rest + i*stride + norm_off);
		} else if (norm_off >= 0) {
			vec4 n = {};
			vert_format_decode(mesh->vert_format, rest + i*stride, vert_semantic_normal, 0, &n);
			norm = XMLoadFloat4((const XMFLOAT4*)&n);
		} else {
			norm = XMVectorZero();
		}
		// Blend the bone matrices unconditionally, then transform once.
		// Zero weights contribute nothing, and mispredictable early-exit
		// branches here cost more than the math they skip (~2x).
		const bone_weight_t *bone = &mesh->skin_data.bone_data[i];
		const matrix* pal = mesh->skin_data.bone_transforms;
		XMMATRIX m0 = XMLoadFloat4x4((XMFLOAT4X4*)&pal[bone->bone_id[0]]);
		XMMATRIX m1 = XMLoadFloat4x4((XMFLOAT4X4*)&pal[bone->bone_id[1]]);
		XMMATRIX m2 = XMLoadFloat4x4((XMFLOAT4X4*)&pal[bone->bone_id[2]]);
		XMMATRIX m3 = XMLoadFloat4x4((XMFLOAT4X4*)&pal[bone->bone_id[3]]);
		float    w0 = bone->weight[0] * (1.0f/255.0f);
		float    w1 = bone->weight[1] * (1.0f/255.0f);
		float    w2 = bone->weight[2] * (1.0f/255.0f);
		float    w3 = bone->weight[3] * (1.0f/255.0f);
		XMMATRIX m;
		m.r[0] = XMVectorAdd(XMVectorAdd(XMVectorScale(m0.r[0], w0), XMVectorScale(m1.r[0], w1)), XMVectorAdd(XMVectorScale(m2.r[0], w2), XMVectorScale(m3.r[0], w3)));
		m.r[1] = XMVectorAdd(XMVectorAdd(XMVectorScale(m0.r[1], w0), XMVectorScale(m1.r[1], w1)), XMVectorAdd(XMVectorScale(m2.r[1], w2), XMVectorScale(m3.r[1], w3)));
		m.r[2] = XMVectorAdd(XMVectorAdd(XMVectorScale(m0.r[2], w0), XMVectorScale(m1.r[2], w1)), XMVectorAdd(XMVectorScale(m2.r[2], w2), XMVectorScale(m3.r[2], w3)));
		m.r[3] = XMVectorAdd(XMVectorAdd(XMVectorScale(m0.r[3], w0), XMVectorScale(m1.r[3], w1)), XMVectorAdd(XMVectorScale(m2.r[3], w2), XMVectorScale(m3.r[3], w3)));
		XMVECTOR new_pos  = XMVector3Transform      (pos,  m);
		XMVECTOR new_norm = XMVector3TransformNormal(norm, m);
		if (pos_f32) {
			xm_store_v3(def + i*stride + pos_off, new_pos);
		} else {
			XMFLOAT4 p;
			XMStoreFloat4(&p, new_pos);
			vert_format_encode(mesh->vert_format, def + i*stride, vert_semantic_position, 0, vec4{p.x, p.y, p.z, 0});
		}
		if (norm_f32) {
			xm_store_v3(def + i*stride + norm_off, new_norm);
		} else if (norm_off >= 0) {
			XMFLOAT4 n;
			XMStoreFloat4(&n, new_norm);
			vert_format_encode(mesh->vert_format, def + i*stride, vert_semantic_normal, 0, vec4{n.x, n.y, n.z, 0});
		}
		min = XMVectorMin(min, new_pos);
		max = XMVectorMax(max, new_pos);
	}
	XMVECTOR center     = XMVectorMultiplyAdd(min, g_XMOneHalf, XMVectorMultiply(max, g_XMOneHalf));
	XMVECTOR dimensions = XMVectorSubtract(max, min);
	mesh->bounds.center     = math_fast_to_vec3(center);
	mesh->bounds.dimensions = math_fast_to_vec3(dimensions);
	_mesh_set_verts(mesh, mesh->skin_data.deformed_verts, mesh->vert_count, mesh->vert_format, false, false);
}

///////////////////////////////////////////

mesh_t mesh_find(const char *id) {
	mesh_t result = (mesh_t)assets_find(id, asset_type_mesh);
	if (result != nullptr) {
		mesh_addref(result);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

void mesh_set_id(mesh_t mesh, const char *id) {
	assets_set_id(&mesh->header, id);
	mesh_update_label(mesh);
}

///////////////////////////////////////////

const char* mesh_get_id(const mesh_t mesh) {
	return mesh->header.id_text;
}

///////////////////////////////////////////

void mesh_update_label(mesh_t mesh) {
#if defined(_DEBUG) || defined(SK_GPU_LABELS)
	if (mesh->header.id_text != nullptr)
		skr_mesh_set_name(&mesh->gpu_mesh, mesh->header.id_text);
#else
	(void)mesh;
#endif
}

///////////////////////////////////////////

void mesh_addref(mesh_t mesh) {
	assets_addref(&mesh->header);
}

///////////////////////////////////////////

asset_state_ mesh_asset_state(const mesh_t mesh) {
	return mesh->header.state;
}

///////////////////////////////////////////

void mesh_on_load(mesh_t mesh, void (*on_load)(mesh_t mesh, void *context), void *context) {
	assets_on_load(&mesh->header, (void(*)(asset_header_t*,void*))on_load, context);
}

///////////////////////////////////////////

void mesh_on_load_remove(mesh_t mesh, void (*on_load)(mesh_t mesh, void *context)) {
	assets_on_load_remove(&mesh->header, (void(*)(asset_header_t*,void*))on_load);
}

///////////////////////////////////////////

mesh_t mesh_create() {
	mesh_t result = (_mesh_t*)assets_allocate(asset_type_mesh);
	vert_format_addref(result->vert_format);
	// Initialize gpu_mesh with empty vertex type — actual vertex format
	// is assigned when vertex data is provided via mesh_set_verts.
	// This allows index-only meshes for vertex-pulling (SV_VertexID).
	skr_mesh_create(nullptr, skr_index_fmt_u32, nullptr, 0, nullptr, 0, &result->gpu_mesh);
	return result;
}

///////////////////////////////////////////

mesh_t mesh_copy(mesh_t mesh) {
	if (mesh == nullptr) {
		log_err("mesh_copy was provided a null mesh!");
		return nullptr;
	}

	mesh_t result = (mesh_t)assets_allocate(asset_type_mesh);
	vert_format_addref(result->vert_format);
	result->gpu_mesh.vert_type  = render_get_default_vert();
	result->gpu_mesh.ind_format = skr_index_fmt_u32;
	result->bounds       = mesh->bounds;
	result->discard_data = mesh->discard_data;
	result->ind_draw     = mesh->ind_draw;

	if (mesh->discard_data) {
		log_err("mesh_copy not yet implemented for meshes with discard data set!");
	} else {
		if (mesh->vert_format == VERT_FORMAT_DEFAULT) {
			mesh_set_verts(result, mesh->verts, mesh->vert_count, false);
		} else {
			int32_t                 component_count = 0;
			const vert_component_t* components      = vert_format_get_components(mesh->vert_format, &component_count);
			mesh_set_verts_fmt(result, components, component_count, mesh->verts, mesh->vert_count, false);
		}
		mesh_set_inds (result, mesh->inds,  mesh->ind_count);
		if (mesh_has_skin(mesh))
			mesh_set_skin_inv(result, mesh->skin_data.bone_data, mesh->vert_count, mesh->skin_data.bone_inverse_transforms, mesh->skin_data.bone_count);
	}

	return result;
}

///////////////////////////////////////////

const mesh_collision_t *mesh_get_collision_data(mesh_t mesh) {
	if (mesh->collision_data.pts != nullptr)
		return &mesh->collision_data;
	if (mesh->discard_data)
		return nullptr;

	// Positions get extracted through the vertex format, so any format
	// with a float3 position supports collision.
	vert_fmt_ pos_fmt   = vert_fmt_none;
	int32_t   pos_count = 0;
	int32_t   pos_off   = vert_format_semantic_offset(mesh->vert_format, vert_semantic_position, 0, &pos_fmt, &pos_count);
	if (pos_off < 0 || pos_fmt != vert_fmt_f32 || pos_count < 3) {
		log_warn("mesh_get_collision_data: mesh collision requires a float3 position component");
		return nullptr;
	}
	const uint8_t* pos    = (const uint8_t*)mesh->verts + pos_off;
	uint32_t       stride = mesh->vert_stride;

	mesh_collision_t &coll = mesh->collision_data;
	coll.pts    = sk_malloc_t(vec3   , mesh->ind_count);
	coll.planes = sk_malloc_t(plane_t, mesh->ind_count/3);

	// memcpy tolerates the unaligned positions packed formats can produce
	for (uint32_t i = 0; i < mesh->ind_count; i++) memcpy(&coll.pts[i], pos + mesh->inds[i]*stride, sizeof(vec3));

	for (uint32_t i = 0; i < mesh->ind_count; i += 3) {
		vec3    dir1   = coll.pts[i+1] - coll.pts[i];
		vec3    dir2   = coll.pts[i+1] - coll.pts[i+2];
		vec3    normal = vec3_normalize( vec3_cross(dir2, dir1) );
		plane_t plane  = { normal, -vec3_dot(coll.pts[i + 1], normal) };
		coll.planes[i/3] = plane;
	}

	return &mesh->collision_data;
}

///////////////////////////////////////////

const mesh_bvh_t *mesh_get_bvh_data(mesh_t mesh) {
	if (mesh->bvh_data != nullptr)
		return mesh->bvh_data;
	if (mesh->discard_data)
		return nullptr;

	// The BVH builds from mesh_get_collision_data, so this handles custom
	// vertex formats too, and comes back null when there's no position.
	mesh->bvh_data = mesh_bvh_create(mesh, 16);

	return mesh->bvh_data;
}

///////////////////////////////////////////

void mesh_release(mesh_t mesh) {
	if (mesh == nullptr)
		return;
	assets_releaseref(&mesh->header);
}

///////////////////////////////////////////

void mesh_destroy(mesh_t mesh) {
	// The gpu mesh's vert_type points into the format registry, destroy it
	// before releasing what may be the format's last reference.
	skr_mesh_destroy(&mesh->gpu_mesh);
	vert_format_release(mesh->vert_format);
	sk_free(mesh->verts);
	sk_free(mesh->inds);
	sk_free(mesh->collision_data.pts   );	// XXX doesn't this fail when no colldata has been created?
	sk_free(mesh->collision_data.planes);
	if (mesh->bvh_data)
		mesh_bvh_destroy(mesh->bvh_data);

	sk_free(mesh->skin_data.bone_data);
	sk_free(mesh->skin_data.bone_inverse_transforms);
	sk_free(mesh->skin_data.bone_transforms);
	sk_free(mesh->skin_data.deformed_verts);

	*mesh = {};
}


///////////////////////////////////////////

void mesh_draw(mesh_t mesh, material_t material, matrix transform, color128 color_linear, render_layer_ layer) {
	render_add_mesh(mesh, material, transform, color_linear, layer);
}

///////////////////////////////////////////

bool32_t mesh_ray_intersect(mesh_t mesh, ray_t model_space_ray, cull_ cull_mode, ray_t *out_pt, uint32_t* out_opt_start_inds) {
	*out_pt = {};
	if (out_opt_start_inds) *out_opt_start_inds = 0;

	vec3 result = {};

	if (mesh == nullptr)
		return false;
	const mesh_collision_t *data = mesh_get_collision_data(mesh);
	if (data == nullptr)
		return false;
	if (!bounds_ray_intersect(mesh->bounds, model_space_ray, &result))
		return false;

	vec3  pt = {};
	float nearest_dist = FLT_MAX;
	for (uint32_t i = 0; i < mesh->ind_count; i+=3) {

		const plane_t& plane = data->planes[i / 3];

		float denom = vec3_dot(model_space_ray.dir, plane.normal);

		if (fabsf(denom) < 1e-6f)
		{
			// Ray direction (almost) perpendicular to plane, no intersection
			continue;
		}

		if ((cull_mode == cull_front && denom < 0) || (cull_mode == cull_back && denom > 0))
		{
			// Front/back-face culling
			// XXX is there a smaller test?
			continue;
		}

		float t_hit = -(vec3_dot(model_space_ray.pos, plane.normal) + plane.d) / denom;

		if (t_hit < 0)
		{
			// Hit behind ray origin
			continue;
		}

		pt = model_space_ray.pos + model_space_ray.dir * t_hit;

		// point in triangle, implementation based on:
		// https://blackpawn.com/texts/pointinpoly/default.html

		// Compute vectors
		vec3 v0 = data->pts[i+1] - data->pts[i];
		vec3 v1 = data->pts[i+2] - data->pts[i];
		vec3 v2 = pt - data->pts[i];

		// Compute dot products
		float dot00 = vec3_dot(v0, v0);
		float dot01 = vec3_dot(v0, v1);
		float dot02 = vec3_dot(v0, v2);
		float dot11 = vec3_dot(v1, v1);
		float dot12 = vec3_dot(v1, v2);

		// Compute barycentric coordinates
		float inv_denom = 1.0f / (dot00 * dot11 - dot01 * dot01);
		float u = (dot11 * dot02 - dot01 * dot12) * inv_denom;
		float v = (dot00 * dot12 - dot01 * dot02) * inv_denom;

		// Check if point is in triangle
		if ((u >= 0) && (v >= 0) && (u + v < 1)) {
			float dist = vec3_magnitude_sq(pt - model_space_ray.pos);
			if (nearest_dist > dist) {
				nearest_dist = dist;
				if (out_opt_start_inds) *out_opt_start_inds = i;
				*out_pt = {pt, data->planes[i / 3].normal};
			}
		}
	}

	return nearest_dist != FLT_MAX;
}

///////////////////////////////////////////

bool32_t mesh_ray_intersect_bvh(mesh_t mesh, ray_t model_space_ray, cull_ cull_mode, ray_t *out_pt, uint32_t* out_start_inds) {
	vec3 result = {};

	const mesh_bvh_t *bvh = mesh_get_bvh_data(mesh);
	if (bvh == nullptr)
		return false;
	if (!bounds_ray_intersect(mesh->bounds, model_space_ray, &result))
		return false;

	return mesh_bvh_intersect(bvh, model_space_ray, out_pt, out_start_inds, cull_mode);
}

///////////////////////////////////////////

bool32_t mesh_get_triangle(mesh_t mesh, uint32_t triangle_index, vert_t* a, vert_t* b, vert_t* c) {
	if (mesh->discard_data) {
		log_err("mesh_get_triangle: can't work with a mesh that doesn't keep data, ensure mesh_get_keep_data() is true");
		return false;
	}
	// Phrased to dodge overflow from a triangle_index near UINT32_MAX
	if (mesh->ind_count < 3 || triangle_index > mesh->ind_count - 3)
		return false;

	if (mesh->vert_format == VERT_FORMAT_DEFAULT) {
		*a = mesh->verts[mesh->inds[triangle_index]];
		*b = mesh->verts[mesh->inds[triangle_index + 1]];
		*c = mesh->verts[mesh->inds[triangle_index + 2]];
		return true;
	}

	// Custom formats decode into vert_t, components the format doesn't
	// have read as vert_create style defaults.
	vert_t* out[3] = {a, b, c};
	for (int32_t i = 0; i < 3; i++) {
		const void* vert = (uint8_t*)mesh->verts + mesh->inds[triangle_index + i] * mesh->vert_stride;
		vec4 pos  = {0,0,0,1};
		vec4 norm = {0,1,0,0};
		vec4 uv   = {0,0,0,0};
		vec4 col  = {1,1,1,1};
		vert_format_decode(mesh->vert_format, vert, vert_semantic_position, 0, &pos );
		vert_format_decode(mesh->vert_format, vert, vert_semantic_normal,   0, &norm);
		vert_format_decode(mesh->vert_format, vert, vert_semantic_texcoord, 0, &uv  );
		vert_format_decode(mesh->vert_format, vert, vert_semantic_color,    0, &col );
		*out[i] = vert_t{
			{pos.x, pos.y, pos.z},
			{norm.x, norm.y, norm.z},
			{uv.x, uv.y},
			color32{
				(uint8_t)roundf(fmaxf(0, fminf(1, col.x)) * 255),
				(uint8_t)roundf(fmaxf(0, fminf(1, col.y)) * 255),
				(uint8_t)roundf(fmaxf(0, fminf(1, col.z)) * 255),
				(uint8_t)roundf(fmaxf(0, fminf(1, col.w)) * 255) } };
	}
	return true;
}

///////////////////////////////////////////

void mesh_gen_cube_vert(int i, const vec3 &size, vec3 &pos, vec3 &norm, vec2 &uv) {
	float neg = (float)((i / 4) % 2 ? -1 : 1);
	int nx  = ((i+24) / 16) % 2;
	int ny  = (i / 8)       % 2;
	int nz  = (i / 16)      % 2;
	int u   = ((i+1) / 2)   % 2; // U: 0,1,1,0
	int v   = (i / 2)       % 2; // V: 0,0,1,1

	uv   = { (float)u, 1.0f-(float)v };
	norm = { nx*neg, ny*neg, nz*neg };
	pos  = {
		size.x * (nx ? neg : ny ? (u?-1:1)*neg : (u?1:-1)*neg),
		size.y * (nx || nz ? (v?1:-1) : neg),
		size.z * (nx ? (u?-1:1)*neg : ny ? (v?1:-1) : neg)
	};
}

///////////////////////////////////////////

mesh_t mesh_gen_plane(vec2 dimensions, vec3 plane_normal, vec3 plane_top_direction, int32_t subdivisions, bool32_t double_sided) {
	vind_t subd   = (vind_t)subdivisions;
	mesh_t result = mesh_create();

	subd = maxi(0,(int32_t)subd) + 2;

	int vert_count = subd*subd;
	int ind_count  = 6*(subd-1)*(subd-1);

	if (double_sided) {
		vert_count *= 2;
		ind_count  *= 2;
	}

	vert_t *verts = sk_malloc_t(vert_t, vert_count);
	vind_t *inds  = sk_malloc_t(vind_t, ind_count );

	vec3 right = vec3_cross(plane_top_direction, plane_normal);
	vec3 up    = vec3_cross(right,               plane_normal);

	// Make vertices
	for (vind_t y = 0; y < subd; y++) {
		float yp = y / (float)(subd-1);
	for (vind_t x = 0; x < subd; x++) {
		float xp = x / (float)(subd-1);

		verts[x + y*subd] = vert_t{ 
			right * ((xp - 0.5f) * dimensions.x) +
			up    * ((yp - 0.5f) * dimensions.y), 
			plane_normal, {xp,yp}, {255,255,255,255} };

		// The flip side of the plane has the same position and UV but a flipped normal
		if (double_sided) {
			verts[x + y*subd + vert_count/2]      = verts[x + y*subd];
			verts[x + y*subd + vert_count/2].norm = -plane_normal;
		}
	} }

	// make indices
	int ind = 0;
	for (vind_t y = 0; y < subd-1; y++) {
	for (vind_t x = 0; x < subd-1; x++) {
			inds[ind++] = (x+1) + (y+1) * subd;
			inds[ind++] = (x+1) +  y    * subd;
			inds[ind++] =  x    +  y    * subd;

			inds[ind++] =  x    + (y+1) * subd;
			inds[ind++] = (x+1) + (y+1) * subd;
			inds[ind++] =  x    +  y    * subd;
	} }

	if (double_sided) {
		for (vind_t y = 0; y < subd-1; y++) {
		for (vind_t x = 0; x < subd-1; x++) {
				// We flip the winding for the flip side
				inds[ind++] = (x+1) +  y    * subd + vert_count/2;
				inds[ind++] = (x+1) + (y+1) * subd + vert_count/2;
				inds[ind++] =  x    +  y    * subd + vert_count/2;

				inds[ind++] = (x+1) + (y+1) * subd + vert_count/2;
				inds[ind++] =  x    + (y+1) * subd + vert_count/2;
				inds[ind++] =  x    +  y    * subd + vert_count/2;
		} }
	}

	mesh_set_data(result, verts, vert_count, inds, ind_count, mesh_data_calc_bounds);

	sk_free(verts);
	sk_free(inds);
	return result;
}

///////////////////////////////////////////

mesh_t mesh_gen_circle(float diameter, vec3 plane_normal, vec3 plane_top_direction, int32_t spokes, bool32_t double_sided) {
	vind_t spoke_count = maxi(3, (int32_t)spokes);
	mesh_t result = mesh_create();

	int vert_count = spoke_count;
	int ind_count  = (spoke_count - 2) * 3;

	if (double_sided) {
		vert_count *= 2;
		ind_count  *= 2;
	}

	vert_t* verts = sk_malloc_t(vert_t, vert_count);
	vind_t* inds  = sk_malloc_t(vind_t, ind_count);

	vec3 right = vec3_cross(plane_top_direction, plane_normal);
	vec3 up    = vec3_cross(right, plane_normal);

	// Make a circle of vertices
	for (vind_t i = 0; i < spoke_count; i++) {
		float angle = i * ((float)M_PI*2.0f / spoke_count);

		vert_t *pt   = &verts[i];
		float radius = diameter / 2;
		float xp     = cosf(angle);
		float yp     = sinf(angle);

		pt->norm = plane_normal;
		pt->pos  = radius * ((right * xp) + (up * yp));
		pt->uv   = {((xp+1)/2),((yp+1)/2)};
		pt->col  = {255,255,255,255};

		// The flip side of the circle has the same position and UV but a flipped normal
		if (double_sided) {
			vert_t* flip_pt = &verts[i + vert_count/2];

			flip_pt->norm = -plane_normal;
			flip_pt->pos  = pt->pos;
			flip_pt->uv   = pt->uv;
			flip_pt->col  = pt->col;
		}
	}

	// No vertex in the center, so we're adding a strip of triangles
	// across the circle instead
	for (vind_t i = 0; i < spoke_count - 2; i++) {
		uint32_t half = i / 2;
		if (i%2 == 0) { // even
			vind_t ind1 = (spoke_count - half) % spoke_count;
			vind_t ind2 = half + 1;
			vind_t ind3 = (spoke_count - 1) - half;

			inds[i*3+2] = ind1;
			inds[i*3+1] = ind2;
			inds[i*3  ] = ind3;

			if (double_sided) {
				// We flip the winding for the flip side
				inds[ind_count/2 + i*3+2] = vert_count/2 + ind2;
				inds[ind_count/2 + i*3+1] = vert_count/2 + ind1;
				inds[ind_count/2 + i*3  ] = vert_count/2 + ind3;
			}
		}
		else { // odd
			vind_t ind1 = half + 1;
			vind_t ind2 = spoke_count - (half + 1);
			vind_t ind3 = half + 2;

			inds[i * 3] = ind1;
			inds[i*3+1] = ind2;
			inds[i*3+2] = ind3;

			if (double_sided) {
				// We flip the winding for the flip side
				inds[ind_count/2 + i*3  ] = vert_count/2 + ind1;
				inds[ind_count/2 + i*3+1] = vert_count/2 + ind3;
				inds[ind_count/2 + i*3+2] = vert_count/2 + ind2;
			}
		}
	}

	mesh_set_data(result, verts, vert_count, inds, ind_count, mesh_data_calc_bounds);

	sk_free(verts);
	sk_free(inds);
	return result;
}

///////////////////////////////////////////

mesh_t mesh_gen_cube(vec3 dimensions, int32_t subdivisions) {
	vind_t subd   = (vind_t)subdivisions;
	mesh_t result = mesh_create();

	subd = maxi((int32_t)0,(int32_t)subd) + 2;

	int vert_count = 6*subd*subd;
	int ind_count  = 6*(subd-1)*(subd-1)*6;
	vert_t *verts = sk_malloc_t(vert_t, vert_count);
	vind_t *inds  = sk_malloc_t(vind_t, ind_count);

	vec3   size   = dimensions / 2;
	int    ind    = 0;
	vind_t offset = 0;
	for (vind_t i = 0; i < 6*4; i+=4) {
		vec3 p1, p2, p3, p4;
		vec3 n1, n2, n3, n4;
		vec2 u1, u2, u3, u4;

		mesh_gen_cube_vert(i,   size, p1, n1, u1);
		mesh_gen_cube_vert(i+1, size, p2, n2, u2);
		mesh_gen_cube_vert(i+2, size, p3, n3, u3);
		mesh_gen_cube_vert(i+3, size, p4, n4, u4);

		offset = (i/4) * (subd)*(subd);
		for (vind_t y = 0; y < subd; y++) {
			float  py    = y / (float)(subd-1);
			vind_t yOff  = offset + y * subd;
			vind_t yOffN = offset + (y+1) * subd;

			vec3 pl = vec3_lerp(p1, p4, py);
			vec3 pr = vec3_lerp(p2, p3, py);
			vec3 nl = vec3_lerp(n1, n4, py);
			vec3 nr = vec3_lerp(n2, n3, py);
			vec2 ul = vec2_lerp(u1, u4, py);
			vec2 ur = vec2_lerp(u2, u3, py);

			for (vind_t x = 0; x < subd; x++) {
				float px = x / (float)(subd-1);
				vind_t  ptIndex = x + yOff;
				vert_t *pt      = &verts[ptIndex];

				pt->pos = vec3_lerp(pl, pr, px);
				pt->norm= vec3_lerp(nl, nr, px);
				pt->uv  = vec2_lerp(ul, ur, px);
				pt->col = {255,255,255,255};

				if (y != subd-1 && x != subd-1) {

					inds[ind++] = (x  ) + yOff;
					inds[ind++] = (x+1) + yOff;
					inds[ind++] = (x+1) + yOffN;

					inds[ind++] = (x  ) + yOff;
					inds[ind++] = (x+1) + yOffN;
					inds[ind++] = (x  ) + yOffN;
				}
			}
		}
	}

	mesh_set_data(result, verts, vert_count, inds, ind_count, mesh_data_calc_bounds);

	sk_free(verts);
	sk_free(inds);
	return result;
}

///////////////////////////////////////////

mesh_t mesh_gen_sphere(float diameter, int32_t subdivisions) {
	vind_t subd   = (vind_t)subdivisions;
	mesh_t result = mesh_create();

	subd = maxi(0,(int32_t)subd) + 2;

	int vert_count = 6*subd*subd;
	int ind_count  = 6*(subd-1)*(subd-1)*6;
	vert_t *verts = sk_malloc_t(vert_t, vert_count);
	vind_t *inds  = sk_malloc_t(vind_t, ind_count);

	vec3   size = vec3_one;
	float  radius = diameter / 2;
	int    ind    = 0;
	vind_t offset = 0;
	for (vind_t i = 0; i < 6*4; i+=4) {
		vec3 p1, p2, p3, p4;
		vec3 n1, n2, n3, n4;
		vec2 u1, u2, u3, u4;

		mesh_gen_cube_vert(i,   size, p1, n1, u1);
		mesh_gen_cube_vert(i+1, size, p2, n2, u2);
		mesh_gen_cube_vert(i+2, size, p3, n3, u3);
		mesh_gen_cube_vert(i+3, size, p4, n4, u4);

		offset = (i/4) * (subd)*(subd);
		for (vind_t y = 0; y < subd; y++) {
			float  py    = y / (float)(subd-1);
			vind_t yOff  = offset + y * subd;
			vind_t yOffN = offset + (y+1) * subd;

			vec3 pl = vec3_lerp(p1, p4, py);
			vec3 pr = vec3_lerp(p2, p3, py);
			vec2 ul = vec2_lerp(u1, u4, py);
			vec2 ur = vec2_lerp(u2, u3, py);

			for (vind_t x = 0; x < subd; x++) {
				float px = x / (float)(subd-1);
				vind_t  ptIndex = x + yOff;
				vert_t *pt = &verts[ptIndex];

				pt->norm= vec3_normalize(vec3_lerp(pl, pr, px));
				pt->pos = pt->norm*radius;
				pt->uv  = vec2_lerp(ul, ur, px);
				pt->col = {255,255,255,255};

				if (y != subd-1 && x != subd-1) {

					inds[ind++] = (x  ) + yOff;
					inds[ind++] = (x+1) + yOff;
					inds[ind++] = (x+1) + yOffN;

					inds[ind++] = (x  ) + yOff;
					inds[ind++] = (x+1) + yOffN;
					inds[ind++] = (x  ) + yOffN;
				}
			}
		}
	}

	mesh_set_data(result, verts, vert_count, inds, ind_count, mesh_data_calc_bounds);

	sk_free(verts);
	sk_free(inds);
	return result;
}

///////////////////////////////////////////

mesh_t mesh_gen_cylinder(float diameter, float depth, vec3 dir, int32_t subdivisions) {
	mesh_t result = mesh_create();
	dir = vec3_normalize(dir);
	float radius = diameter / 2;

	vind_t subd = (vind_t)subdivisions;
	int vert_count = (subdivisions+1) * 4 + 2;
	int ind_count  = subdivisions * 12;
	vert_t *verts = sk_malloc_t(vert_t, vert_count);
	vind_t *inds  = sk_malloc_t(vind_t, ind_count);

	// Calculate any perpendicular vector
	vec3 perp = vec3{dir.z, dir.z, -dir.x-dir.y};
	if (vec3_magnitude_sq(perp) == 0)
		perp = vec3{-dir.y-dir.z, dir.x, dir.x};

	vec3 axis_x = vec3_normalize(vec3_cross(dir, perp));
	vec3 axis_y = vec3_normalize(vec3_cross(dir, axis_x));
	vec3 z_off  = dir * (depth / 2.f);
	vind_t ind = 0;

	for (vind_t i = 0; i <= subd; i++) {
		float u   = ((float)i / subd);
		float ang = u * (float)M_PI * 2;
		float x   = cosf(ang);
		float y   = sinf(ang);
		vec3 normal  = axis_x * x + axis_y * y;
		vec3 top_pos = normal*radius + z_off;
		vec3 bot_pos = normal*radius - z_off;

		// strip first
		verts[i * 4  ] = { top_pos,  normal, {u,0}, {255,255,255,255} };
		verts[i * 4+1] = { bot_pos,  normal, {u,1}, {255,255,255,255} };
		// now circular faces
		verts[i * 4+2] = { top_pos,  dir,    {u,0}, {255,255,255,255} };
		verts[i * 4+3] = { bot_pos, -dir,    {u,1}, {255,255,255,255} };

		if (i == subd) continue;

		vind_t in = (i + 1) % (subd+1);
		// Top slice
		inds[ind++] = i  * 4 + 2;
		inds[ind++] = in * 4 + 2;
		inds[ind++] = (subd+1) * 4;
		// Bottom slice
		inds[ind++] = (subd+1) * 4+1;
		inds[ind++] = in * 4 + 3;
		inds[ind++] = i  * 4 + 3;
		// Now edge strip quad
		inds[ind++] = in * 4+1;
		inds[ind++] = in * 4;
		inds[ind++] = i  * 4;
		inds[ind++] = i  * 4+1;
		inds[ind++] = in * 4+1;
		inds[ind++] = i  * 4;
	}
	// center points for the circle
	verts[(subdivisions+1)*4]   = {  z_off,  dir, {0.5f,0.01f}, {255,255,255,255} };
	verts[(subdivisions+1)*4+1] = { -z_off, -dir, {0.5f,0.99f}, {255,255,255,255} };

	mesh_set_data(result, verts, vert_count, inds, ind_count, mesh_data_calc_bounds);

	sk_free(verts);
	sk_free(inds);
	return result;
}

///////////////////////////////////////////

// Bottom always at origin, top at dir*depth
mesh_t mesh_gen_cone(float diameter, float depth, vec3 dir, int32_t subdivisions) {
	mesh_t result = mesh_create();
	dir = vec3_normalize(dir);
	float radius = diameter / 2;

	vind_t subd = (vind_t)subdivisions;
	int vert_count = (subdivisions+1) * 4 + 2;
	int ind_count  = subdivisions * 12;
	vert_t *verts = sk_malloc_t(vert_t, vert_count);
	vind_t *inds  = sk_malloc_t(vind_t, ind_count);

	// Calculate any perpendicular vector
	vec3 perp = vec3{dir.z, dir.z, -dir.x-dir.y};
	if (vec3_magnitude_sq(perp) == 0)
		perp = vec3{-dir.y-dir.z, dir.x, dir.x};

	vec3 axis_x = vec3_normalize(vec3_cross(dir, perp));
	vec3 axis_y = vec3_normalize(vec3_cross(dir, axis_x));
	vec3 z_off  = dir * (depth / 2.f);
	vec3 top_pos = dir * depth;
	vind_t ind = 0;

	for (vind_t i = 0; i <= subd; i++) {
		float u   = ((float)i / subd);
		float ang = u * (float)M_PI * 2;
		float x   = cosf(ang);
		float y   = sinf(ang);
		vec3 normal  = axis_x * x + axis_y * y;
		vec3 bot_pos = normal*radius;

		// strip first
		verts[i * 4  ] = { top_pos,  normal, {u,0}, {255,255,255,255} };
		verts[i * 4+1] = { bot_pos,  normal, {u,1}, {255,255,255,255} };
		// now circular faces
		verts[i * 4+2] = { top_pos,  dir,    {u,0}, {255,255,255,255} };
		verts[i * 4+3] = { bot_pos, -dir,    {u,1}, {255,255,255,255} };

		if (i == subd) continue;

		vind_t in = (i + 1) % (subd+1);
		// Top slice
		inds[ind++] = i  * 4 + 2;
		inds[ind++] = in * 4 + 2;
		inds[ind++] = (subd+1) * 4;
		// Bottom slice
		inds[ind++] = (subd+1) * 4+1;
		inds[ind++] = in * 4 + 3;
		inds[ind++] = i  * 4 + 3;
		// Now edge strip quad
		inds[ind++] = in * 4+1;
		inds[ind++] = in * 4;
		inds[ind++] = i  * 4;
		inds[ind++] = i  * 4+1;
		inds[ind++] = in * 4+1;
		inds[ind++] = i  * 4;
	}
	// center points for the circle
	verts[(subdivisions+1)*4]   = {  z_off,  dir, {0.5f,0.01f}, {255,255,255,255} };
	verts[(subdivisions+1)*4+1] = { vec3{}, -dir, {0.5f,0.99f}, {255,255,255,255} };

	mesh_set_data(result, verts, vert_count, inds, ind_count, mesh_data_calc_bounds);

	sk_free(verts);
	sk_free(inds);
	return result;
}

///////////////////////////////////////////

mesh_t mesh_gen_rounded_cube(vec3 dimensions, float edge_radius, int32_t subdivisions) {
	vind_t subd   = (vind_t)subdivisions;
	mesh_t result = mesh_create();

	subd = maxi(0,(int32_t)subd) + 2;
	if (subd % 2 == 1) // need an even number of subdivisions
		subd += 1;

	vind_t  vert_count = 6*subd*subd;
	vind_t  ind_count  = 6*(subd-1)*(subd-1)*6;
	vert_t *verts = sk_malloc_t(vert_t, vert_count);
	vind_t *inds  = sk_malloc_t(vind_t, ind_count );

	vec3   off = (dimensions / 2) - vec3_one*edge_radius;
	vec3   size = vec3_one;
	float  radius = edge_radius;
	vind_t ind    = 0;
	vind_t offset = 0;
	for (vind_t i = 0; i < 6*4; i+=4) {
		vec3 p1, p2, p3, p4;
		vec3 n1, n2, n3, n4;
		vec2 u1, u2, u3, u4;

		mesh_gen_cube_vert(i,   size, p1, n1, u1);
		mesh_gen_cube_vert(i+1, size, p2, n2, u2);
		mesh_gen_cube_vert(i+2, size, p3, n3, u3);
		mesh_gen_cube_vert(i+3, size, p4, n4, u4);

		float sizeU = vec3_magnitude((p4 - p1) * (dimensions/2));
		float sizeV = vec3_magnitude((p2 - p1) * (dimensions/2));

		offset = (i/4) * (subd)*(subd);
		vind_t x, y;
		for (vind_t sy = 0; sy < subd; sy++) {
			bool first_half_y = sy < subd / 2;
			y = first_half_y ? sy : sy-1;
			vec3 stretchA  = first_half_y ? p1 : p4;
			vec3 stretchB  = first_half_y ? p2 : p3;
			float stretchV = (radius*2)/sizeV;
			float offV     = first_half_y ? 0 : 1-((radius/sizeV)*2);
			
			float py    = y / (float)(subd-2);
			float pv    = py * stretchV + offV;
			vind_t yOff  = offset + sy * subd;
			vind_t yOffN = offset + (sy+1) * subd;
			
			vec3 pl = vec3_lerp(p1, p4, py);
			vec3 pr = vec3_lerp(p2, p3, py);
			vec2 ul = vec2_lerp(u1, u4, pv);
			vec2 ur = vec2_lerp(u2, u3, pv);

			for (vind_t sx = 0; sx < subd; sx++) {
				bool first_half_x = sx < subd / 2;
				x = first_half_x ? sx : sx-1;
				vec3  stretch = first_half_x ? stretchA : stretchB;
				float stretchU = (radius*2)/sizeU;
				float offU     = first_half_x ? 0 : 1-((radius/sizeU)*2);

				float px      = x / (float)(subd-2);
				float pu      = px * stretchU + offU;
				int   ptIndex = sx + yOff;
				vert_t *pt    = &verts[ptIndex];

				pt->norm= vec3_normalize(vec3_lerp(pl, pr, px));
				pt->pos = pt->norm*radius + stretch*off;
				pt->uv = vec2_lerp(ul, ur, pu);
				pt->col = {255,255,255,255};

				if (sy != subd-1 && sx != subd-1) {
					inds[ind++] = (sx  ) + yOff;
					inds[ind++] = (sx+1) + yOff;
					inds[ind++] = (sx+1) + yOffN;

					inds[ind++] = (sx  ) + yOff;
					inds[ind++] = (sx+1) + yOffN;
					inds[ind++] = (sx  ) + yOffN;
				}
			}
		}
	}

	mesh_set_data(result, verts, vert_count, inds, ind_count, mesh_data_calc_bounds);

	sk_free(verts);
	sk_free(inds);
	return result;
}

} // namespace sk