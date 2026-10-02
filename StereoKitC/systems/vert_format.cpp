// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

#include "vert_format.h"
#include "../sk_memory.h"
#include "../platforms/platform.h"
#include "../libraries/ferr_hash.h"
#include "../libraries/ferr_thread.h"
#include "../libraries/atomic_util.h"

#include <string.h>
#include <math.h>

namespace sk {

// Entries are individually allocated into a fixed size table, so entry
// addresses and slots stay stable, and readers never take the lock.
#define VERT_FORMAT_MAX 256

struct vert_format_entry_t {
	uint64_t          hash;
	vert_component_t* components;
	int32_t           component_count;
	uint32_t          stride;
	skr_vert_type_t   skr_type;
	int32_t           refs; // atomic
};

struct vert_format_state_t {
	vert_format_entry_t** entries; // zero-ref slots are reused, ids stay valid forever
	int32_t               count;
	ft_mutex_t            lock;    // guards entry creation/destruction, refcounts are atomic
};
static vert_format_state_t local = {};

///////////////////////////////////////////

int32_t vert_fmt_size(vert_fmt_ format) {
	switch (format) {
	case vert_fmt_f32:
	case vert_fmt_i32:
	case vert_fmt_u32:            return 4;
	case vert_fmt_f16:
	case vert_fmt_i16:
	case vert_fmt_u16:
	case vert_fmt_i16_normalized:
	case vert_fmt_u16_normalized: return 2;
	case vert_fmt_i8:
	case vert_fmt_u8:
	case vert_fmt_i8_normalized:
	case vert_fmt_u8_normalized:  return 1;
	default:                      return 0;
	}
}

///////////////////////////////////////////

static skr_vertex_fmt_ vert_fmt_to_skr(vert_fmt_ format) {
	switch (format) {
	case vert_fmt_f32:            return skr_vertex_fmt_f32;
	case vert_fmt_f16:            return skr_vertex_fmt_f16;
	case vert_fmt_i32:            return skr_vertex_fmt_i32;
	case vert_fmt_i16:            return skr_vertex_fmt_i16;
	case vert_fmt_i8:             return skr_vertex_fmt_i8;
	case vert_fmt_i16_normalized: return skr_vertex_fmt_i16_normalized;
	case vert_fmt_i8_normalized:  return skr_vertex_fmt_i8_normalized;
	case vert_fmt_u32:            return skr_vertex_fmt_ui32;
	case vert_fmt_u16:            return skr_vertex_fmt_ui16;
	case vert_fmt_u8:             return skr_vertex_fmt_ui8;
	case vert_fmt_u16_normalized: return skr_vertex_fmt_ui16_normalized;
	case vert_fmt_u8_normalized:  return skr_vertex_fmt_ui8_normalized;
	default:                      return skr_vertex_fmt_none;
	}
}

///////////////////////////////////////////

static skr_semantic_ vert_semantic_to_skr(vert_semantic_ semantic) {
	switch (semantic) {
	case vert_semantic_position:     return skr_semantic_position;
	case vert_semantic_normal:       return skr_semantic_normal;
	case vert_semantic_texcoord:     return skr_semantic_texcoord;
	case vert_semantic_color:        return skr_semantic_color;
	case vert_semantic_tangent:      return skr_semantic_tangent;
	case vert_semantic_binormal:     return skr_semantic_binormal;
	case vert_semantic_blendweight:  return skr_semantic_blendweight;
	case vert_semantic_blendindices: return skr_semantic_blendindices;
	case vert_semantic_psize:        return skr_semantic_psize;
	default:                         return skr_semantic_none;
	}
}

///////////////////////////////////////////

static bool vert_format_validate(const vert_component_t* components, int32_t count) {
	if (components == nullptr || count <= 0) {
		log_err("Vertex formats need at least one component!");
		return false;
	}
	for (int32_t i = 0; i < count; i++) {
		const vert_component_t* c = &components[i];
		if (vert_fmt_size((vert_fmt_)c->format) == 0)                                { log_errf("Vertex component %d has an invalid format!",   i); return false; }
		if (c->count < 1 || c->count > 4)                                            { log_errf("Vertex component %d count must be 1-4!",       i); return false; }
		if (vert_semantic_to_skr((vert_semantic_)c->semantic) == skr_semantic_none)  { log_errf("Vertex component %d has an invalid semantic!", i); return false; }
		for (int32_t p = 0; p < i; p++) {
			if (components[p].semantic == c->semantic && components[p].semantic_slot == c->semantic_slot) {
				log_errf("Vertex components %d and %d share a semantic, semantics may only appear once per format!", p, i);
				return false;
			}
		}
	}
	return true;
}

///////////////////////////////////////////

static vert_format_entry_t* vert_format_entry_create(const vert_component_t* components, int32_t count, uint64_t hash) {
	skr_vert_component_t skr_components[16];
	if (count > (int32_t)_countof(skr_components)) {
		log_errf("Vertex formats support at most %d components!", (int32_t)_countof(skr_components));
		return nullptr;
	}
	uint32_t stride = 0;
	for (int32_t i = 0; i < count; i++) {
		skr_components[i] = {};
		skr_components[i].format        = vert_fmt_to_skr     ((vert_fmt_     )components[i].format);
		skr_components[i].count         = components[i].count;
		skr_components[i].semantic      = vert_semantic_to_skr((vert_semantic_)components[i].semantic);
		skr_components[i].semantic_slot = components[i].semantic_slot;
		stride += vert_fmt_size((vert_fmt_)components[i].format) * components[i].count;
	}

	vert_format_entry_t* entry = sk_malloc_zero_t(vert_format_entry_t, 1);
	if (skr_vert_type_create(skr_components, count, &entry->skr_type) != skr_err_success) {
		log_err("Failed to create GPU vertex type!");
		sk_free(entry);
		return nullptr;
	}
	entry->hash            = hash;
	entry->components      = sk_malloc_t(vert_component_t, count);
	entry->component_count = count;
	entry->stride          = stride;
	entry->refs            = 0;
	memcpy(entry->components, components, sizeof(vert_component_t) * count);
	return entry;
}

///////////////////////////////////////////

// Core of vert_format_ref, the caller holds the registry lock.
static int32_t vert_format_ref_unlocked(const vert_component_t* components, int32_t count) {
	uint64_t hash = hash_fnv64_data(components, sizeof(vert_component_t) * count);

	int32_t free_slot = -1;
	for (int32_t i = 0; i < local.count; i++) {
		vert_format_entry_t* entry = local.entries[i];
		if (entry == nullptr) { if (free_slot < 0) free_slot = i; continue; }
		if (entry->hash == hash && entry->component_count == count &&
			memcmp(entry->components, components, sizeof(vert_component_t) * count) == 0) {
			atomic_increment(&entry->refs);
			return i;
		}
	}
	if (free_slot < 0 && local.count >= VERT_FORMAT_MAX) {
		log_errf("Too many unique vertex formats, the limit is %d!", VERT_FORMAT_MAX);
		return -1;
	}

	vert_format_entry_t* entry = vert_format_entry_create(components, count, hash);
	if (entry == nullptr)
		return -1;
	entry->refs = 1;

	int32_t id = free_slot;
	if (id < 0) { id = local.count; local.count += 1; }
	local.entries[id] = entry;
	return id;
}

///////////////////////////////////////////

bool vert_format_sys_init() {
	local         = {};
	local.entries = sk_malloc_zero_t(vert_format_entry_t*, VERT_FORMAT_MAX);
	local.lock    = ft_mutex_create();

	// Id 0 is the built-in vert_t format, and the reference taken here is
	// the registry's own, keeping it alive for the registry's whole life.
	vert_component_t default_components[] = {
		vert_component(vert_semantic_position, vert_fmt_f32,           3),
		vert_component(vert_semantic_normal,   vert_fmt_f32,           3),
		vert_component(vert_semantic_texcoord, vert_fmt_f32,           2),
		vert_component(vert_semantic_color,    vert_fmt_u8_normalized, 4),
	};
	ft_mutex_lock  (local.lock);
	int32_t id = vert_format_ref_unlocked(default_components, _countof(default_components));
	ft_mutex_unlock(local.lock);
	return id == VERT_FORMAT_DEFAULT;
}

///////////////////////////////////////////

void vert_format_sys_shutdown() {
	if (local.entries == nullptr) return;
	for (int32_t i = 0; i < local.count; i++) {
		vert_format_entry_t* entry = local.entries[i];
		if (entry == nullptr) continue;
		// The registry holds one reference to the default format itself.
		int32_t expected = i == VERT_FORMAT_DEFAULT ? 1 : 0;
		if (entry->refs > expected)
			log_warnf("Vertex format %d leaked with %d references!", i, entry->refs - expected);
		skr_vert_type_destroy(&entry->skr_type);
		sk_free(entry->components);
		sk_free(entry);
	}
	sk_free(local.entries);
	ft_mutex_destroy(&local.lock);
	local = {};
}

///////////////////////////////////////////

int32_t vert_format_ref(const vert_component_t* components, int32_t count) {
	if (!vert_format_validate(components, count))
		return -1;

	ft_mutex_lock(local.lock);
	int32_t id = vert_format_ref_unlocked(components, count);
	ft_mutex_unlock(local.lock);
	return id;
}

///////////////////////////////////////////

void vert_format_addref(int32_t id) {
	// Lock-free is safe here, callers only addref formats that are alive.
	atomic_increment(&local.entries[id]->refs);
}

///////////////////////////////////////////

void vert_format_release(int32_t id) {
	if (id < 0) return;
	vert_format_entry_t* entry = local.entries[id];
	if (atomic_decrement(&entry->refs) > 0) return;

	// On zero, re-check under the lock, a concurrent vert_format_ref may
	// have resurrected this entry through deduplication.
	ft_mutex_lock(local.lock);
	if (local.entries[id] == entry && entry->refs <= 0) {
		skr_vert_type_destroy(&entry->skr_type);
		sk_free(entry->components);
		sk_free(entry);
		local.entries[id] = nullptr;
	}
	ft_mutex_unlock(local.lock);
}

///////////////////////////////////////////

const skr_vert_type_t* vert_format_get_skr(int32_t id) {
	return &local.entries[id]->skr_type;
}

///////////////////////////////////////////

const vert_component_t* vert_format_get_components(int32_t id, int32_t* out_count) {
	vert_format_entry_t* entry = local.entries[id];
	*out_count = entry->component_count;
	return entry->components;
}

///////////////////////////////////////////

uint32_t vert_format_get_stride(int32_t id) {
	return local.entries[id]->stride;
}

///////////////////////////////////////////

// Subnormal halfs are vanishingly small and flush to zero here.
static float vert_fmt_half_to_float(uint16_t h) {
	uint32_t sign = (uint32_t)(h & 0x8000) << 16;
	uint32_t exp  = (h >> 10) & 0x1F;
	uint32_t man  =  h        & 0x3FF;
	uint32_t bits =
		exp == 0  ? sign :
		exp == 31 ? sign | 0x7F800000        | (man << 13)
		          : sign | ((exp + 112) << 23) | (man << 13);
	float result;
	memcpy(&result, &bits, sizeof(result));
	return result;
}

// Round to nearest, overflow becomes infinity, underflow becomes zero,
// and NaN stays NaN.
static uint16_t vert_fmt_float_to_half(float f) {
	uint32_t bits;
	memcpy(&bits, &f, sizeof(bits));
	uint32_t sign = (bits >> 16) & 0x8000;
	int32_t  exp  = (int32_t)((bits >> 23) & 0xFF) - 127 + 15;
	uint32_t man  = bits & 0x7FFFFF;
	if (exp >= 31) return (uint16_t)(sign | 0x7C00 | (man != 0 ? 0x200 : 0));
	if (exp <= 0 ) return (uint16_t)sign;
	uint16_t result = (uint16_t)(sign | (exp << 10) | (man >> 13));
	// Round bit carries up through the exponent correctly on overflow.
	if (man & 0x1000) result += 1;
	return result;
}

///////////////////////////////////////////

// Unaligned safe reads/writes, memcpy compiles down to plain loads/stores.
static float vert_fmt_decode_element(vert_fmt_ format, const uint8_t* at) {
	switch (format) {
	case vert_fmt_f32:            { float    v; memcpy(&v, at, 4); return v; }
	case vert_fmt_f16:            { uint16_t v; memcpy(&v, at, 2); return vert_fmt_half_to_float(v); }
	case vert_fmt_i32:            { int32_t  v; memcpy(&v, at, 4); return (float)v; }
	case vert_fmt_i16:            { int16_t  v; memcpy(&v, at, 2); return (float)v; }
	case vert_fmt_i8:             { return (float)*(int8_t*)at; }
	case vert_fmt_i16_normalized: { int16_t  v; memcpy(&v, at, 2); return fmaxf(v / 32767.0f, -1); }
	case vert_fmt_i8_normalized:  { return fmaxf(*(int8_t*)at / 127.0f, -1); }
	case vert_fmt_u32:            { uint32_t v; memcpy(&v, at, 4); return (float)v; }
	case vert_fmt_u16:            { uint16_t v; memcpy(&v, at, 2); return (float)v; }
	case vert_fmt_u8:             { return (float)*at; }
	case vert_fmt_u16_normalized: { uint16_t v; memcpy(&v, at, 2); return v / 65535.0f; }
	case vert_fmt_u8_normalized:  { return *at / 255.0f; }
	default:                      { return 0; }
	}
}

// Out of range values clamp to the format's limits, casting them directly
// is undefined behavior. 32 bit limits clamp in doubles, their integer
// bounds aren't exactly representable as floats.
static void vert_fmt_encode_element(vert_fmt_ format, uint8_t* at, float value) {
	switch (format) {
	case vert_fmt_f32:            { memcpy(at, &value, 4); } break;
	case vert_fmt_f16:            { uint16_t v = vert_fmt_float_to_half(value); memcpy(at, &v, 2); } break;
	case vert_fmt_i32:            { int32_t  v = (int32_t )fmax (fmin ((double)value, 2147483647.0), -2147483648.0); memcpy(at, &v, 4); } break;
	case vert_fmt_i16:            { int16_t  v = (int16_t )fmaxf(fminf(value,  32767.0f), -32768.0f); memcpy(at, &v, 2); } break;
	case vert_fmt_i8:             { *(int8_t*)at = (int8_t )fmaxf(fminf(value,    127.0f),   -128.0f); } break;
	case vert_fmt_i16_normalized: { int16_t  v = (int16_t )roundf(fmaxf(-1, fminf(1, value)) * 32767); memcpy(at, &v, 2); } break;
	case vert_fmt_i8_normalized:  { *(int8_t*)at = (int8_t )roundf(fmaxf(-1, fminf(1, value)) * 127); } break;
	case vert_fmt_u32:            { uint32_t v = (uint32_t)fmax (fmin ((double)value, 4294967295.0), 0.0); memcpy(at, &v, 4); } break;
	case vert_fmt_u16:            { uint16_t v = (uint16_t)fmaxf(fminf(value,  65535.0f), 0.0f); memcpy(at, &v, 2); } break;
	case vert_fmt_u8:             { *at = (uint8_t)fmaxf(fminf(value, 255.0f), 0.0f); } break;
	case vert_fmt_u16_normalized: { uint16_t v = (uint16_t)roundf(fmaxf(0, fminf(1, value)) * 65535); memcpy(at, &v, 2); } break;
	case vert_fmt_u8_normalized:  { *at = (uint8_t)roundf(fmaxf(0, fminf(1, value)) * 255); } break;
	default: break;
	}
}

///////////////////////////////////////////

bool vert_format_decode(int32_t id, const void* vertex, vert_semantic_ semantic, int32_t semantic_slot, vec4* ref_value) {
	vert_fmt_ fmt    = vert_fmt_none;
	int32_t   count  = 0;
	int32_t   offset = vert_format_semantic_offset(id, semantic, semantic_slot, &fmt, &count);
	if (offset < 0) return false;

	const uint8_t* at   = (const uint8_t*)vertex + offset;
	int32_t        size = vert_fmt_size(fmt);
	float*         out  = &ref_value->x;
	for (int32_t i = 0; i < count; i++)
		out[i] = vert_fmt_decode_element(fmt, at + i*size);
	return true;
}

///////////////////////////////////////////

bool vert_format_encode(int32_t id, void* vertex, vert_semantic_ semantic, int32_t semantic_slot, vec4 value) {
	vert_fmt_ fmt    = vert_fmt_none;
	int32_t   count  = 0;
	int32_t   offset = vert_format_semantic_offset(id, semantic, semantic_slot, &fmt, &count);
	if (offset < 0) return false;

	uint8_t*     at   = (uint8_t*)vertex + offset;
	int32_t      size = vert_fmt_size(fmt);
	const float* in   = &value.x;
	for (int32_t i = 0; i < count; i++)
		vert_fmt_encode_element(fmt, at + i*size, in[i]);
	return true;
}

///////////////////////////////////////////

int32_t vert_format_semantic_offset(int32_t id, vert_semantic_ semantic, int32_t semantic_slot, vert_fmt_* out_opt_fmt, int32_t* out_opt_count) {
	vert_format_entry_t* entry  = local.entries[id];
	int32_t              offset = 0;
	for (int32_t i = 0; i < entry->component_count; i++) {
		const vert_component_t* c = &entry->components[i];
		if (c->semantic == semantic && c->semantic_slot == semantic_slot) {
			if (out_opt_fmt  ) *out_opt_fmt   = (vert_fmt_)c->format;
			if (out_opt_count) *out_opt_count = c->count;
			return offset;
		}
		offset += vert_fmt_size((vert_fmt_)c->format) * c->count;
	}
	return -1;
}

///////////////////////////////////////////

int32_t mesh_fmt_stride(const vert_component_t* format, int32_t component_count) {
	int32_t stride = 0;
	for (int32_t i = 0; i < component_count; i++)
		stride += vert_fmt_size((vert_fmt_)format[i].format) * format[i].count;
	return stride;
}

} // namespace sk
