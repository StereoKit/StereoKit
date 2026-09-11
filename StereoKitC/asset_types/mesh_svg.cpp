#include "../stereokit.h"
#include "../sk_memory.h"
#include "../libraries/array.h"
#include "../libraries/nanosvg.h"
#include "../utils/triangulate.h"
#include "mesh.h"
#include "mesh_svg.h"

#include <math.h>
#include <string.h>

namespace sk {

typedef struct svg_build_t {
	array_t<vec2>     pts;   // image space positions for the current shape
	array_t<uint32_t> inds;  // triangles for the current shape
	array_t<vec2>     poly;  // flattened path scratch
	array_t<uint32_t> sides; // stroke scratch, four side indices per polyline point
	array_t<int32_t>  ring_starts;
	array_t<int32_t>  ring_counts;
	array_t<vert_t>   verts;
	array_t<vind_t>   mesh_inds;
	float             tol;   // max distance a flattened curve strays from the true one
	float             fan_step; // radians per triangle for the current stroke's round joins and caps
	float             w;
	float             h;
	bool              translucent; // any vertex with alpha below one
} svg_build_t;

///////////////////////////////////////////

static void svg_flatten_cubic(array_t<vec2>* ref_pts, vec2 p1, vec2 p2, vec2 p3, vec2 p4, float tol_sq, int32_t level) {
	if (level > 10) { ref_pts->add(p4); return; }

	// A chord too short to measure against, like the closing segment of a
	// path, is flat when its control points stay within tolerance.
	vec2  d  = p4 - p1;
	float dd = vec2_magnitude_sq(d);
	if (dd < tol_sq) {
		if (vec2_distance_sq(p2, p1) < tol_sq && vec2_distance_sq(p3, p4) < tol_sq) { ref_pts->add(p4); return; }
	} else {
		float d2 = fabsf((p2.x-p4.x)*d.y - (p2.y-p4.y)*d.x);
		float d3 = fabsf((p3.x-p4.x)*d.y - (p3.y-p4.y)*d.x);
		if ((d2+d3)*(d2+d3) < tol_sq * dd) { ref_pts->add(p4); return; }
	}

	vec2 p12   = (p1  +p2  )*0.5f;
	vec2 p23   = (p2  +p3  )*0.5f;
	vec2 p34   = (p3  +p4  )*0.5f;
	vec2 p123  = (p12 +p23 )*0.5f;
	vec2 p234  = (p23 +p34 )*0.5f;
	vec2 p1234 = (p123+p234)*0.5f;
	svg_flatten_cubic(ref_pts, p1,    p12,  p123, p1234, tol_sq, level+1);
	svg_flatten_cubic(ref_pts, p1234, p234, p34,  p4,    tol_sq, level+1);
}

///////////////////////////////////////////

// Flattens a path to a polyline with no repeated points. A closed path
// also drops a final point that lands back on the first.
static void svg_flatten_path(svg_build_t* b, const NSVGpath* path, bool closed) {
	b->poly.clear();
	if (path->npts < 1) return;

	b->poly.add({ path->pts[0], path->pts[1] });
	for (int32_t i = 0; i+3 < path->npts; i += 3) {
		const float* p = &path->pts[i*2];
		svg_flatten_cubic(&b->poly, {p[0],p[1]}, {p[2],p[3]}, {p[4],p[5]}, {p[6],p[7]}, b->tol*b->tol, 0);
	}

	float   min_dist_sq = b->tol*b->tol*0.01f;
	int32_t count       = 1;
	for (int32_t i = 1; i < b->poly.count; i++) {
		if (vec2_distance_sq(b->poly[i], b->poly[count-1]) > min_dist_sq)
			b->poly[count++] = b->poly[i];
	}
	if (closed && count > 1 && vec2_distance_sq(b->poly[count-1], b->poly[0]) <= min_dist_sq)
		count -= 1;
	b->poly.count = count;
}

///////////////////////////////////////////

static color32 svg_color(uint32_t abgr, float opacity) {
	color128 c = {
		( abgr      & 0xff) / 255.f,
		((abgr>> 8) & 0xff) / 255.f,
		((abgr>>16) & 0xff) / 255.f,
		((abgr>>24) & 0xff) / 255.f * opacity };
	return color_to_32(color_to_linear(c));
}

///////////////////////////////////////////

static float svg_gradient_t(const NSVGgradient* g, bool radial, vec2 p) {
	const float* x  = g->xform;
	float        gx = p.x*x[0] + p.y*x[2] + x[4];
	float        gy = p.x*x[1] + p.y*x[3] + x[5];
	return radial ? sqrtf(gx*gx + gy*gy) : gy;
}

///////////////////////////////////////////

// Stops interpolate in gamma space, matching SVG, spread is always pad.
static color32 svg_gradient_color(const NSVGgradient* g, float t, float opacity) {
	if (g->nstops < 1) return { 255, 255, 255, (uint8_t)(opacity*255) };

	int32_t i = 0;
	while (i < g->nstops-1 && t > g->stops[i+1].offset) i++;
	const NSVGgradientStop* s0 = &g->stops[i];
	const NSVGgradientStop* s1 = &g->stops[i < g->nstops-1 ? i+1 : i];

	float span = s1->offset - s0->offset;
	float f    = span > 0 ? fminf(fmaxf((t - s0->offset) / span, 0), 1) : 0;
	float c[4];
	for (int32_t ch = 0; ch < 4; ch++) {
		float a = (s0->color >> (ch*8)) & 0xff;
		float b = (s1->color >> (ch*8)) & 0xff;
		c[ch] = (a + (b-a)*f) / 255.f;
	}
	color128 col = { c[0], c[1], c[2], c[3] * opacity };
	return color_to_32(color_to_linear(col));
}

///////////////////////////////////////////

static color32 svg_paint_color(const NSVGpaint* paint, float opacity, vec2 p) {
	switch (paint->type) {
	case NSVG_PAINT_COLOR:           return svg_color         (paint->color, opacity);
	case NSVG_PAINT_LINEAR_GRADIENT: return svg_gradient_color(paint->gradient, svg_gradient_t(paint->gradient, false, p), opacity);
	case NSVG_PAINT_RADIAL_GRADIENT: return svg_gradient_color(paint->gradient, svg_gradient_t(paint->gradient, true,  p), opacity);
	default:                         return { 255, 255, 255, 255 };
	}
}

///////////////////////////////////////////

static bool svg_paint_visible(const NSVGpaint* paint) {
	if (paint->type == NSVG_PAINT_NONE) return false;
	if (paint->type == NSVG_PAINT_COLOR && (paint->color >> 24) == 0) return false;
	return true;
}

///////////////////////////////////////////

// Both triangles sharing an edge must produce the same point, so the lerp
// always runs from the lower index.
static uint32_t svg_split_point(svg_build_t* b, uint32_t ia, uint32_t ib, float da, float db) {
	if (ia > ib) { uint32_t ti = ia; ia = ib; ib = ti; float td = da; da = db; db = td; }
	vec2 a = b->pts[ia];
	vec2 c = b->pts[ib];
	return (uint32_t)b->pts.add(a + (c-a) * (da / (da-db)));
}

///////////////////////////////////////////

// Cuts the shape's triangles along the gradient's stop lines so each stop
// color lands on a vertex, otherwise a 4 vert rect would lose them all.
static void svg_split_linear(svg_build_t* b, const NSVGgradient* g) {
	const float eps = 0.00001f;
	for (int32_t s = -1; s < g->nstops; s++) {
		float split = s < 0 ? 0 : (s == g->nstops-1 ? 1 : g->stops[s].offset);
		if (s >= 0 && (split <= 0 || split >= 1)) continue;

		int32_t tri_count = b->inds.count / 3;
		for (int32_t t = 0; t < tri_count; t++) {
			uint32_t i[3] = { b->inds[t*3], b->inds[t*3+1], b->inds[t*3+2] };
			float    d[3];
			int32_t  pos = 0, neg = 0, zero = -1;
			for (int32_t k = 0; k < 3; k++) {
				d[k] = svg_gradient_t(g, false, b->pts[i[k]]) - split;
				if      (d[k] >  eps) pos += 1;
				else if (d[k] < -eps) neg += 1;
				else                  zero = k;
			}
			if (pos == 0 || neg == 0) continue;

			if (zero >= 0) {
				// One vertex on the line, split the opposite edge in two.
				int32_t k1 = (zero+1)%3, k2 = (zero+2)%3;
				uint32_t x = svg_split_point(b, i[k1], i[k2], d[k1], d[k2]);
				b->inds[t*3+k2] = x;
				b->inds.add(i[zero]); b->inds.add(x); b->inds.add(i[k2]);
				continue;
			}

			// The lone vertex is on its own side, the other two share the far
			// side. Cut both edges leaving the lone vertex.
			int32_t lone = 0;
			for (int32_t k = 0; k < 3; k++)
				if ((d[k] > 0) == (pos == 1)) lone = k;
			int32_t  ka = (lone+1)%3, kb = (lone+2)%3;
			uint32_t x1 = svg_split_point(b, i[lone], i[ka], d[lone], d[ka]);
			uint32_t x2 = svg_split_point(b, i[lone], i[kb], d[lone], d[kb]);
			b->inds[t*3] = i[lone]; b->inds[t*3+1] = x1; b->inds[t*3+2] = x2;
			b->inds.add(x1); b->inds.add(i[ka]); b->inds.add(i[kb]);
			b->inds.add(x1); b->inds.add(i[kb]); b->inds.add(x2);
		}
	}
}

///////////////////////////////////////////

// A radial gradient needs vertices across the shape, not just around it,
// so long edges in gradient space get split until the falloff has room.
static void svg_split_radial(svg_build_t* b, const NSVGgradient* g) {
	const float max_len_sq = 0.25f * 0.25f;
	for (int32_t pass = 0; pass < 4; pass++) {
		int32_t tri_count = b->inds.count / 3;
		bool    split_any = false;
		for (int32_t t = 0; t < tri_count; t++) {
			uint32_t i[3] = { b->inds[t*3], b->inds[t*3+1], b->inds[t*3+2] };
			vec2     gp[3];
			for (int32_t k = 0; k < 3; k++) {
				vec2         p = b->pts[i[k]];
				const float* x = g->xform;
				gp[k] = { p.x*x[0] + p.y*x[2] + x[4], p.x*x[1] + p.y*x[3] + x[5] };
			}
			// The longest edge splits, keeping the cut point shared with the
			// neighbor by always lerping from the lower index.
			int32_t longest = 0;
			float   len_sq  = 0;
			for (int32_t k = 0; k < 3; k++) {
				float l = vec2_distance_sq(gp[k], gp[(k+1)%3]);
				if (l > len_sq) { len_sq = l; longest = k; }
			}
			if (len_sq <= max_len_sq) continue;
			int32_t  ka = longest, kb = (longest+1)%3, kc = (longest+2)%3;
			uint32_t x  = svg_split_point(b, i[ka], i[kb], 1, -1);
			b->inds[t*3+kb] = x;
			b->inds.add(x); b->inds.add(i[kb]); b->inds.add(i[kc]);
			split_any = true;
		}
		if (!split_any) break;
	}
}

///////////////////////////////////////////

static void svg_emit(svg_build_t* b, const NSVGpaint* paint, float opacity) {
	if (b->inds.count == 0) return;
	if (paint->type == NSVG_PAINT_LINEAR_GRADIENT) svg_split_linear(b, paint->gradient);
	if (paint->type == NSVG_PAINT_RADIAL_GRADIENT) svg_split_radial(b, paint->gradient);

	bool     solid = paint->type == NSVG_PAINT_COLOR;
	color32  color = solid ? svg_color(paint->color, opacity) : color32{};
	uint32_t base  = (uint32_t)b->verts.count;
	for (int32_t i = 0; i < b->pts.count; i++) {
		vec2    p = b->pts[i];
		color32 c = solid ? color : svg_paint_color(paint, opacity, p);
		if (c.a < 255) b->translucent = true;
		b->verts.add({ {p.x, p.y, 0}, {0,0,-1}, {p.x/b->w, p.y/b->h}, c });
	}
	for (int32_t i = 0; i < b->inds.count; i++)
		b->mesh_inds.add(base + b->inds[i]);
}

///////////////////////////////////////////

static void svg_add_fill(svg_build_t* b, const NSVGshape* shape) {
	b->pts        .clear();
	b->inds       .clear();
	b->ring_starts.clear();
	b->ring_counts.clear();
	for (const NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
		svg_flatten_path(b, path, true);
		if (b->poly.count < 3) continue;
		b->ring_starts.add(b->pts.count);
		b->ring_counts.add(b->poly.count);
		b->pts.add_range(b->poly.data, b->poly.count);
	}
	triangulate_rings(b->pts.data, b->ring_starts.data, b->ring_counts.data, b->ring_starts.count, shape->fillRule == NSVG_FILLRULE_EVENODD, &b->inds);
	svg_emit(b, &shape->fill, shape->opacity);
}

///////////////////////////////////////////

static void svg_tri(svg_build_t* b, uint32_t i0, uint32_t i1, uint32_t i2) {
	b->inds.add(i0); b->inds.add(i1); b->inds.add(i2);
}

///////////////////////////////////////////

// Arc of triangles around center from the point at 'from' to the point at
// 'to', both existing indices, sweeping counter-clockwise or not.
static void svg_fan(svg_build_t* b, uint32_t center, uint32_t from, uint32_t to, bool ccw) {
	vec2  c     = b->pts[center];
	vec2  vf    = b->pts[from] - c;
	vec2  vt    = b->pts[to  ] - c;
	float cross = vf.x*vt.y - vf.y*vt.x;
	float dot   = vf.x*vt.x + vf.y*vt.y;

	// Most joins on a curve turn by less than one step, and for those the
	// cross product is the angle, no trig needed for a single triangle.
	if (dot > 0 && fabsf(cross) <= b->fan_step * dot && (ccw ? cross >= 0 : cross <= 0)) {
		svg_tri(b, center, from, to);
		return;
	}

	float sweep = atan2f(cross, dot);
	if (ccw  && sweep < 0) sweep += 6.2831853f;
	if (!ccw && sweep > 0) sweep -= 6.2831853f;

	int32_t count = b->fan_step > 0 ? (int32_t)ceilf(fabsf(sweep) / b->fan_step) : 1;
	count = count < 1 ? 1 : (count > 64 ? 64 : count);

	// Each point is the previous one rotated by a fixed step.
	float    step = sweep / count;
	float    cs   = cosf(step);
	float    sn   = sinf(step);
	vec2     spoke = vf;
	uint32_t prev  = from;
	for (int32_t i = 1; i <= count; i++) {
		uint32_t cur = to;
		if (i < count) {
			spoke = { spoke.x*cs - spoke.y*sn, spoke.x*sn + spoke.y*cs };
			cur   = (uint32_t)b->pts.add(c + spoke);
		}
		svg_tri(b, center, prev, cur);
		prev = cur;
	}
}

///////////////////////////////////////////

// Strokes one polyline with vertices shared along each side. A join shares
// a single point per side where its miter fits, otherwise the outer side
// gets a point per segment with a bevel or round wedge between them.
static void svg_stroke_poly(svg_build_t* b, const NSVGshape* shape, bool closed, float hw) {
	int32_t count = b->poly.count;
	b->sides.clear();
	b->sides.add_empties(count * 4);
	uint32_t* sides = b->sides.data; // in_left, in_right, out_left, out_right

	for (int32_t j = 0; j < count; j++) {
		vec2 v       = b->poly[j];
		bool has_in  = closed || j > 0;
		bool has_out = closed || j < count-1;
		vec2 d_in    = has_in  ? v - b->poly[(j+count-1)%count] : vec2{};
		vec2 d_out   = has_out ? b->poly[(j+1)%count] - v       : vec2{};
		float l_in   = vec2_magnitude(d_in);
		float l_out  = vec2_magnitude(d_out);
		if (has_in)  d_in  = d_in  / l_in;
		if (has_out) d_out = d_out / l_out;
		vec2 n_in    = vec2{ -d_in .y, d_in .x } * hw;
		vec2 n_out   = vec2{ -d_out.y, d_out.x } * hw;
		uint32_t* s  = &sides[j*4];

		// Path ends only have one side to offset from.
		if (!has_in || !has_out) {
			vec2 d = has_in ? d_in  : d_out;
			vec2 n = has_in ? n_in  : n_out;
			if (shape->strokeLineCap == NSVG_CAP_SQUARE) v = has_in ? v + d*hw : v - d*hw;
			s[0] = s[2] = (uint32_t)b->pts.add(v + n);
			s[1] = s[3] = (uint32_t)b->pts.add(v - n);
			continue;
		}

		float turn = d_in.x*d_out.y - d_in.y*d_out.x;
		vec2  m    = n_in + n_out;
		float ml   = vec2_magnitude(m);
		float cos  = ml > 0 ? vec2_dot(m/ml, n_in)/hw : 0;
		if (fabsf(turn) < 0.000001f || cos <= 0) {
			// Straight through, or a full reversal that has no join to make.
			s[0] = s[2] = (uint32_t)b->pts.add(v + n_in);
			s[1] = s[3] = (uint32_t)b->pts.add(v - n_in);
			continue;
		}

		// A left turn tucks the left side in, the right side opens up.
		float   miter_len = hw / cos;
		vec2    miter     = (m/ml) * miter_len;
		int32_t inner     = turn > 0 ? 0 : 1;
		int32_t outer     = 1 - inner;
		float   inner_sgn = inner == 0 ? 1.f : -1.f;
		float   outer_sgn = -inner_sgn;

		// The inner miter overshoots short segments, then each segment keeps
		// its own point and the quads simply overlap.
		bool shared_inner = miter_len <= fminf(l_in, l_out);
		if (shared_inner) {
			s[inner] = s[inner+2] = (uint32_t)b->pts.add(v + miter*inner_sgn);
		} else {
			s[inner  ] = (uint32_t)b->pts.add(v + n_in *inner_sgn);
			s[inner+2] = (uint32_t)b->pts.add(v + n_out*inner_sgn);
		}

		if (shape->strokeLineJoin == NSVG_JOIN_MITER && miter_len <= shape->miterLimit * hw) {
			s[outer] = s[outer+2] = (uint32_t)b->pts.add(v + miter*outer_sgn);
			continue;
		}

		// With a shared inner point the two quads meet there, so the wedge
		// between them has to reach all the way across to it.
		uint32_t center = (uint32_t)b->pts.add(v);
		uint32_t apex   = shared_inner ? s[inner] : center;
		s[outer  ] = (uint32_t)b->pts.add(v + n_in *outer_sgn);
		s[outer+2] = (uint32_t)b->pts.add(v + n_out*outer_sgn);
		if (shape->strokeLineJoin == NSVG_JOIN_ROUND) svg_fan(b, center, s[outer], s[outer+2], turn > 0);
		if (shape->strokeLineJoin != NSVG_JOIN_ROUND || shared_inner) svg_tri(b, apex, s[outer], s[outer+2]);
	}

	int32_t seg_count = closed ? count : count-1;
	for (int32_t j = 0; j < seg_count; j++) {
		const uint32_t* a = &sides[j*4];
		const uint32_t* c = &sides[((j+1)%count)*4];
		svg_tri(b, a[2], a[3], c[1]);
		svg_tri(b, a[2], c[1], c[0]);
	}

	if (!closed && shape->strokeLineCap == NSVG_CAP_ROUND) {
		uint32_t* first = &sides[0];
		uint32_t* last  = &sides[(count-1)*4];
		svg_fan(b, (uint32_t)b->pts.add(b->poly[0]),       first[0], first[1], true);
		svg_fan(b, (uint32_t)b->pts.add(b->poly[count-1]), last [1], last [0], true);
	}
}

///////////////////////////////////////////

static void svg_add_stroke(svg_build_t* b, const NSVGshape* shape) {
	float hw = shape->strokeWidth * 0.5f;
	if (hw <= 0) return;

	b->fan_step = 2 * acosf(fmaxf(0, 1 - b->tol/hw));
	b->pts .clear();
	b->inds.clear();
	for (const NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
		bool closed = path->closed != 0;
		svg_flatten_path(b, path, closed);
		if (b->poly.count < 2) continue;
		if (b->poly.count == 2) closed = false;
		svg_stroke_poly(b, shape, closed, hw);
	}
	svg_emit(b, &shape->stroke, shape->opacity);
}

///////////////////////////////////////////

static bool svg_build(const NSVGimage* img, const svg_options_t* options, mesh_load_t* out_mesh, bool32_t* out_translucent) {
	svg_build_t b = {};
	b.w   = img->width;
	b.h   = img->height;
	b.tol = fmaxf(b.w, b.h) * options->tolerance;
	if (b.w <= 0 || b.h <= 0) {
		log_warn("SVG has no size, skipping.");
		return false;
	}

	// Everything shares one depth, and the default depth test only passes
	// the first triangle rasterized at a pixel. Emitting in reverse document
	// order keeps SVG's painter's order for opaque materials.
	array_t<const NSVGshape*> shapes = {};
	for (const NSVGshape* shape = img->shapes; shape != nullptr; shape = shape->next)
		shapes.add(shape);
	for (int32_t i = 0; i < shapes.count; i++) {
		const NSVGshape* shape = shapes[options->reverse_order ? shapes.count-1-i : i];
		if ((shape->flags & NSVG_FLAGS_VISIBLE) == 0) continue;
		if (options->reverse_order) {
			if (svg_paint_visible(&shape->stroke)) svg_add_stroke(&b, shape);
			if (svg_paint_visible(&shape->fill  )) svg_add_fill  (&b, shape);
		} else {
			if (svg_paint_visible(&shape->fill  )) svg_add_fill  (&b, shape);
			if (svg_paint_visible(&shape->stroke)) svg_add_stroke(&b, shape);
		}
	}
	shapes.free();

	// Center the image and face -Z like Text and UI do, which puts SVG's +x
	// along -X and its +y along +Y. Front faces wind clockwise seen from +Z.
	// A real world size on the root is honored, otherwise the height becomes
	// one meter, matching sprites.
	bool  physical = img->heightUnits == NSVG_UNITS_MM || img->heightUnits == NSVG_UNITS_CM || img->heightUnits == NSVG_UNITS_IN
	              || img->widthUnits  == NSVG_UNITS_MM || img->widthUnits  == NSVG_UNITS_CM || img->widthUnits  == NSVG_UNITS_IN;
	float unit = physical ? 0.0254f / 96.f : 1.0f / b.h;
	for (int32_t i = 0; i < b.verts.count; i++) {
		vec3* p = &b.verts[i].pos;
		*p = { (b.w*0.5f - p->x) * unit, (b.h*0.5f - p->y) * unit, 0 };
	}
	int32_t kept = 0;
	for (int32_t i = 0; i+2 < b.mesh_inds.count; i += 3) {
		vind_t i0 = b.mesh_inds[i], i1 = b.mesh_inds[i+1], i2 = b.mesh_inds[i+2];
		vec3   p0 = b.verts[i0].pos;
		vec3   p1 = b.verts[i1].pos;
		vec3   p2 = b.verts[i2].pos;
		// Float here on purpose, it matches what the GPU will see.
		float cross = (p1.x-p0.x)*(p2.y-p0.y) - (p1.y-p0.y)*(p2.x-p0.x);
		if (cross == 0) continue;
		if (cross >  0) { vind_t t = i1; i1 = i2; i2 = t; }
		b.mesh_inds[kept++] = i0;
		b.mesh_inds[kept++] = i1;
		b.mesh_inds[kept++] = i2;
	}
	b.mesh_inds.count = kept;

	b.pts        .free();
	b.inds       .free();
	b.poly       .free();
	b.sides      .free();
	b.ring_starts.free();
	b.ring_counts.free();

	// The caller takes over the vertex and index memory.
	out_mesh->verts      = b.verts.data;
	out_mesh->vert_count = b.verts.count;
	out_mesh->inds       = b.mesh_inds.data;
	out_mesh->ind_count  = b.mesh_inds.count;
	if (out_translucent != nullptr) *out_translucent = b.translucent;
	return b.verts.count > 0;
}

///////////////////////////////////////////

bool svg_build_mem(const void* svg_data, size_t data_size, const char* name, svg_options_t options, mesh_load_t* out_mesh, bool32_t* out_translucent) {
	// nanosvg edits the text while parsing, and wants a terminator.
	array_t<char> text = {};
	text.add_range((const char*)svg_data, (int32_t)data_size);
	text.add('\0');

	for (const char* c = strstr(text.data, "<text"); c != nullptr; c = strstr(c+5, "<text")) {
		if (c[5] == ' ' || c[5] == '\t' || c[5] == '\n' || c[5] == '\r' || c[5] == '/' || c[5] == '>') {
			log_warnf("SVG <text> elements detected, please bake your text instead! (%s)", name);
			break;
		}
	}

	NSVGimage* img = nsvgParse(text.data, "px", 96);
	text.free();
	if (img == nullptr) {
		log_warnf("Failed to parse SVG: %s", name);
		return false;
	}

	bool built = svg_build(img, &options, out_mesh, out_translucent);
	nsvgDelete(img);
	if (!built) log_warnf("SVG produced no geometry: %s", name);
	return built;
}

}
