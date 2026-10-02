#include "triangulate.h"

#include <math.h>
#include <float.h>
#include <stdlib.h>

namespace sk {

typedef struct tri_ring_t {
	int32_t start;
	int32_t count;
	float   area;   // signed, positive is counter-clockwise
	vec2    min;
	vec2    max;
	vec2    inner;  // a point strictly inside, for testing against other rings
	int32_t parent; // outer ring this hole belongs to
	bool    is_hole;
	bool    dropped;
} tri_ring_t;

typedef struct tri_node_t {
	int32_t idx;
	int32_t prev;
	int32_t next;
	bool    reflex;
	bool    removed;
} tri_node_t;

// Uniform grid used two ways. Over ring bounding boxes, so classifying a
// ring only tests the rings around it. Over reflex vertices, so an ear test
// only visits the ones near its triangle. Reflex vertices can only become
// convex as ears are clipped, never the reverse, so that grid is built once.
typedef struct tri_grid_t {
	vec2    min;
	vec2    inv_cell;
	int32_t cols;
	int32_t rows;
	array_t<int32_t> cell_start; // cols*rows+1 entries into items
	array_t<int32_t> items;      // node indices, grouped by cell
} tri_grid_t;

///////////////////////////////////////////

// Double keeps near-collinear runs of points from flipping between convex
// and reflex on float noise.
static double tri_cross(vec2 a, vec2 b, vec2 c) {
	return ((double)b.x-a.x)*((double)c.y-a.y) - ((double)b.y-a.y)*((double)c.x-a.x);
}

///////////////////////////////////////////

static float tri_ring_area(const vec2* pts, int32_t start, int32_t count) {
	float area = 0;
	for (int32_t i = 0, j = count-1; i < count; j = i++)
		area += pts[start+j].x*pts[start+i].y - pts[start+i].x*pts[start+j].y;
	return area * 0.5f;
}

///////////////////////////////////////////

static bool tri_ring_contains(const vec2* pts, const tri_ring_t* ring, vec2 p) {
	if (p.x < ring->min.x || p.x > ring->max.x || p.y < ring->min.y || p.y > ring->max.y)
		return false;
	bool inside = false;
	for (int32_t i = 0, j = ring->count-1; i < ring->count; j = i++) {
		vec2 a = pts[ring->start+i];
		vec2 b = pts[ring->start+j];
		if ((a.y > p.y) != (b.y > p.y) && p.x < (b.x-a.x) * (p.y-a.y) / (b.y-a.y) + a.x)
			inside = !inside;
	}
	return inside;
}

///////////////////////////////////////////

// Points sitting exactly on a triangle vertex don't count, bridged holes
// duplicate vertices and those must not block their own ears.
static bool tri_point_in(vec2 a, vec2 b, vec2 c, vec2 p) {
	if ((p.x == a.x && p.y == a.y) || (p.x == b.x && p.y == b.y) || (p.x == c.x && p.y == c.y))
		return false;
	return tri_cross(a, b, p) >= 0 && tri_cross(b, c, p) >= 0 && tri_cross(c, a, p) >= 0;
}

///////////////////////////////////////////

// A point just inside the ring at the apex of its first ear. A vertex won't
// do, rings that touch put it right on the neighbor's boundary, and the
// ear's centroid won't either, a hole ring can sit inside the ear.
static vec2 tri_ring_inner_point(const vec2* pts, int32_t start, int32_t count, float area) {
	double sign = area > 0 ? 1 : -1;
	for (int32_t i = 0; i < count; i++) {
		vec2 a = pts[start + (i+count-1)%count];
		vec2 b = pts[start + i];
		vec2 c = pts[start + (i+1)%count];
		if (tri_cross(a, b, c) * sign <= 0) continue;

		bool blocked = false;
		for (int32_t j = 0; j < count && !blocked; j++)
			blocked = sign > 0 ? tri_point_in(a, b, c, pts[start+j]) : tri_point_in(a, c, b, pts[start+j]);
		if (!blocked) return b + ((a + b + c) * (1.f/3.f) - b) * (1.f/64.f);
	}
	return pts[start];
}

///////////////////////////////////////////

static bool tri_is_reflex(const vec2* pts, const tri_node_t* nodes, int32_t n) {
	return tri_cross(pts[nodes[nodes[n].prev].idx], pts[nodes[n].idx], pts[nodes[nodes[n].next].idx]) < 0;
}

///////////////////////////////////////////

// Adds a ring as its own cyclic loop of nodes, returns the first node.
static int32_t tri_add_loop(array_t<tri_node_t>* nodes, int32_t start, int32_t count, bool reverse) {
	int32_t first = nodes->count;
	for (int32_t i = 0; i < count; i++) {
		int32_t idx = reverse ? start + (count-1-i) : start + i;
		nodes->add({ idx, first + (i+count-1)%count, first + (i+1)%count, false, false });
	}
	return first;
}

///////////////////////////////////////////

// Joins a hole loop into the main loop with a zero width bridge, following
// the visibility test from Eberly's "Triangulation by Ear Clipping".
static bool tri_bridge_hole(const vec2* pts, array_t<tri_node_t>* nodes, int32_t main_head, int32_t hole_head) {
	tri_node_t* n = nodes->data;

	// The hole's rightmost vertex, it can always see the main loop to its
	// right.
	int32_t m = hole_head;
	for (int32_t c = n[hole_head].next; c != hole_head; c = n[c].next)
		if (pts[n[c].idx].x > pts[n[m].idx].x) m = c;
	vec2 mp = pts[n[m].idx];

	// Cast a ray to +x, find the closest edge it crosses.
	float   best_x = FLT_MAX;
	int32_t best_a = -1;
	int32_t c = main_head;
	do {
		vec2 a = pts[n[c].idx];
		vec2 b = pts[n[n[c].next].idx];
		if ((a.y <= mp.y) != (b.y <= mp.y)) {
			float x = a.x + (mp.y-a.y) * (b.x-a.x) / (b.y-a.y);
			if (x >= mp.x && x < best_x) { best_x = x; best_a = c; }
		}
		c = n[c].next;
	} while (c != main_head);
	if (best_a < 0) return false;

	int32_t best_b = n[best_a].next;
	int32_t p      = pts[n[best_a].idx].x > pts[n[best_b].idx].x ? best_a : best_b;
	vec2    ip     = { best_x, mp.y };
	vec2    pp     = pts[n[p].idx];

	// Any reflex vertex inside the triangle (m, intersection, p) blocks the
	// view, the one closest to the ray direction is visible instead. The
	// point test wants a counter-clockwise triangle, p may be on either side.
	bool  p_above = pp.y > mp.y;
	vec2  tmin = { mp.x, fminf(mp.y, pp.y) };
	vec2  tmax = { fmaxf(ip.x, pp.x), fmaxf(mp.y, pp.y) };
	float best_cos  = -FLT_MAX;
	float best_dist =  FLT_MAX;
	c = main_head;
	do {
		vec2 rp = pts[n[c].idx];
		if (c != p && rp.x >= tmin.x && rp.x <= tmax.x && rp.y >= tmin.y && rp.y <= tmax.y && tri_is_reflex(pts, n, c) && (p_above ? tri_point_in(mp, ip, pp, rp) : tri_point_in(mp, pp, ip, rp))) {
			vec2  d    = rp - mp;
			float dist = sqrtf(d.x*d.x + d.y*d.y);
			float cos  = dist > 0 ? d.x / dist : 1;
			if (cos > best_cos || (cos == best_cos && dist < best_dist)) {
				best_cos  = cos;
				best_dist = dist;
				p         = c;
			}
		}
		c = n[c].next;
	} while (c != main_head);

	// Splice: ..., p, m, ..hole.., m2, p2, p.next, ...
	tri_node_t m_copy = n[m];
	tri_node_t p_copy = n[p];
	int32_t    m2     = nodes->add(m_copy);
	int32_t    p2     = nodes->add(p_copy);
	n = nodes->data;
	int32_t m_prev = n[m].prev;
	int32_t p_next = n[p].next;
	n[m_prev].next = m2;
	n[m2    ].prev = m_prev;
	n[m2    ].next = p2;
	n[p2    ].prev = m2;
	n[p2    ].next = p_next;
	n[p_next].prev = p2;
	n[p     ].next = m;
	n[m     ].prev = p;
	return true;
}

///////////////////////////////////////////

static int32_t tri_grid_cell(const tri_grid_t* grid, vec2 p) {
	int32_t cx = (int32_t)fminf(fmaxf((p.x - grid->min.x) * grid->inv_cell.x, 0), (float)grid->cols-1);
	int32_t cy = (int32_t)fminf(fmaxf((p.y - grid->min.y) * grid->inv_cell.y, 0), (float)grid->rows-1);
	return cx + cy*grid->cols;
}

///////////////////////////////////////////

static void tri_grid_size(tri_grid_t* grid, vec2 min, vec2 max, int32_t item_count) {
	vec2  size = { fmaxf(max.x-min.x, FLT_EPSILON), fmaxf(max.y-min.y, FLT_EPSILON) };
	float side = sqrtf(size.x * size.y / fmaxf((float)item_count, 1.f));
	grid->min      = min;
	grid->cols     = (int32_t)fminf(fmaxf(ceilf(size.x / side), 1), 256);
	grid->rows     = (int32_t)fminf(fmaxf(ceilf(size.y / side), 1), 256);
	grid->inv_cell = { grid->cols / size.x, grid->rows / size.y };
	grid->cell_start.clear();
	grid->cell_start.add_empties(grid->cols * grid->rows + 1);
}

///////////////////////////////////////////

// Each ring lands in every cell its bounding box touches.
static void tri_ring_grid_build(tri_grid_t* grid, const array_t<tri_ring_t>* rings) {
	vec2    min   = {  FLT_MAX,  FLT_MAX };
	vec2    max   = { -FLT_MAX, -FLT_MAX };
	int32_t count = 0;
	for (int32_t i = 0; i < rings->count; i++) {
		const tri_ring_t* r = &rings->data[i];
		if (r->dropped) continue;
		min = { fminf(min.x, r->min.x), fminf(min.y, r->min.y) };
		max = { fmaxf(max.x, r->max.x), fmaxf(max.y, r->max.y) };
		count += 1;
	}
	tri_grid_size(grid, min, max, count);

	for (int32_t pass = 0; pass < 2; pass++) {
		if (pass == 1) {
			for (int32_t i = 1; i < grid->cell_start.count; i++)
				grid->cell_start[i] += grid->cell_start[i-1];
			grid->items.clear();
			if (grid->cell_start.last() > 0) grid->items.add_empties(grid->cell_start.last());
		}
		array_t<int32_t> fill = {};
		if (pass == 1) fill.add_range(grid->cell_start.data, grid->cell_start.count);
		for (int32_t i = 0; i < rings->count; i++) {
			const tri_ring_t* r = &rings->data[i];
			if (r->dropped) continue;
			int32_t c0 = tri_grid_cell(grid, r->min);
			int32_t c1 = tri_grid_cell(grid, r->max);
			for (int32_t y = c0 / grid->cols; y <= c1 / grid->cols; y++) {
			for (int32_t x = c0 % grid->cols; x <= c1 % grid->cols; x++) {
				int32_t cell = x + y*grid->cols;
				if (pass == 0) grid->cell_start[cell+1] += 1;
				else           grid->items[fill[cell]++] = i;
			} }
		}
		fill.free();
	}
}

///////////////////////////////////////////

static void tri_grid_build(tri_grid_t* grid, const vec2* pts, const tri_node_t* nodes, int32_t head) {
	vec2    min    = {  FLT_MAX,  FLT_MAX };
	vec2    max    = { -FLT_MAX, -FLT_MAX };
	int32_t reflex = 0;
	int32_t c      = head;
	do {
		vec2 p = pts[nodes[c].idx];
		min = { fminf(min.x, p.x), fminf(min.y, p.y) };
		max = { fmaxf(max.x, p.x), fmaxf(max.y, p.y) };
		if (nodes[c].reflex) reflex += 1;
		c = nodes[c].next;
	} while (c != head);

	// Counting sort of the reflex nodes into cells.
	tri_grid_size(grid, min, max, reflex);
	c = head;
	do {
		if (nodes[c].reflex) grid->cell_start[tri_grid_cell(grid, pts[nodes[c].idx]) + 1] += 1;
		c = nodes[c].next;
	} while (c != head);
	for (int32_t i = 1; i < grid->cell_start.count; i++)
		grid->cell_start[i] += grid->cell_start[i-1];

	grid->items.clear();
	if (reflex > 0) grid->items.add_empties(reflex);
	array_t<int32_t> fill = {};
	fill.add_range(grid->cell_start.data, grid->cell_start.count);
	c = head;
	do {
		if (nodes[c].reflex) grid->items[fill[tri_grid_cell(grid, pts[nodes[c].idx])]++] = c;
		c = nodes[c].next;
	} while (c != head);
	fill.free();
}

///////////////////////////////////////////

static bool tri_ear_blocked(const tri_grid_t* grid, const vec2* pts, const tri_node_t* nodes, int32_t prev, int32_t cur, int32_t next) {
	vec2 a = pts[nodes[prev].idx];
	vec2 b = pts[nodes[cur ].idx];
	vec2 c = pts[nodes[next].idx];
	vec2 min = { fminf(fminf(a.x, b.x), c.x), fminf(fminf(a.y, b.y), c.y) };
	vec2 max = { fmaxf(fmaxf(a.x, b.x), c.x), fmaxf(fmaxf(a.y, b.y), c.y) };
	int32_t c0 = tri_grid_cell(grid, min);
	int32_t c1 = tri_grid_cell(grid, max);
	int32_t x0 = c0 % grid->cols, y0 = c0 / grid->cols;
	int32_t x1 = c1 % grid->cols, y1 = c1 / grid->cols;

	for (int32_t y = y0; y <= y1; y++) {
	for (int32_t x = x0; x <= x1; x++) {
		int32_t cell = x + y*grid->cols;
		for (int32_t i = grid->cell_start[cell]; i < grid->cell_start[cell+1]; i++) {
			int32_t r = grid->items[i];
			if (r == prev || r == cur || r == next || nodes[r].removed || !nodes[r].reflex) continue;
			if (tri_point_in(a, b, c, pts[nodes[r].idx])) return true;
		}
	} }
	return false;
}

///////////////////////////////////////////

static void tri_earclip(const vec2* pts, tri_node_t* nodes, int32_t head, int32_t count, tri_grid_t* grid, array_t<uint32_t>* ref_indices) {
	int32_t c = head;
	do {
		nodes[c].reflex = tri_is_reflex(pts, nodes, c);
		c = nodes[c].next;
	} while (c != head);
	tri_grid_build(grid, pts, nodes, head);

	int32_t remaining = count;
	int32_t fails     = 0;
	int32_t cur       = head;
	while (remaining > 3) {
		int32_t prev   = nodes[cur].prev;
		int32_t next   = nodes[cur].next;
		bool    is_ear = !nodes[cur].reflex && !tri_ear_blocked(grid, pts, nodes, prev, cur, next);

		// A full lap without an ear means the input is degenerate somewhere,
		// clipping anyway keeps this terminating.
		if (!is_ear && fails < remaining) {
			fails += 1;
			cur    = next;
			continue;
		}

		vec2 a  = pts[nodes[prev].idx];
		vec2 b  = pts[nodes[cur ].idx];
		vec2 cp = pts[nodes[next].idx];
		if (tri_cross(a, b, cp) > 0) {
			ref_indices->add((uint32_t)nodes[prev].idx);
			ref_indices->add((uint32_t)nodes[cur ].idx);
			ref_indices->add((uint32_t)nodes[next].idx);
		}
		nodes[cur ].removed = true;
		nodes[prev].next    = next;
		nodes[next].prev    = prev;
		nodes[prev].reflex  = tri_is_reflex(pts, nodes, prev);
		nodes[next].reflex  = tri_is_reflex(pts, nodes, next);
		remaining -= 1;
		fails      = 0;
		cur        = next;
	}

	int32_t prev = nodes[cur].prev;
	int32_t next = nodes[cur].next;
	if (tri_cross(pts[nodes[prev].idx], pts[nodes[cur].idx], pts[nodes[next].idx]) > 0) {
		ref_indices->add((uint32_t)nodes[prev].idx);
		ref_indices->add((uint32_t)nodes[cur ].idx);
		ref_indices->add((uint32_t)nodes[next].idx);
	}
}

///////////////////////////////////////////

static int tri_hole_compare(const void* a, const void* b) {
	float fa = ((const tri_ring_t*)a)->max.x;
	float fb = ((const tri_ring_t*)b)->max.x;
	return (fa < fb) - (fa > fb);
}

///////////////////////////////////////////

void triangulate_rings(const vec2* points, const int32_t* ring_starts, const int32_t* ring_counts, int32_t ring_count, bool even_odd, array_t<uint32_t>* ref_indices) {
	array_t<tri_ring_t> rings = {};
	rings.resize(ring_count);
	for (int32_t i = 0; i < ring_count; i++) {
		tri_ring_t r = { ring_starts[i], ring_counts[i], 0, { FLT_MAX, FLT_MAX }, { -FLT_MAX, -FLT_MAX }, {}, -1, false, false };
		r.area    = r.count >= 3 ? tri_ring_area(points, r.start, r.count) : 0;
		r.dropped = fabsf(r.area) < FLT_EPSILON;
		r.inner   = r.dropped ? points[r.start] : tri_ring_inner_point(points, r.start, r.count, r.area);
		for (int32_t p = 0; p < r.count; p++) {
			vec2 pt = points[r.start+p];
			r.min = { fminf(r.min.x, pt.x), fminf(r.min.y, pt.y) };
			r.max = { fmaxf(r.max.x, pt.x), fmaxf(r.max.y, pt.y) };
		}
		rings.add(r);
	}

	tri_grid_t ring_grid = {};
	tri_ring_grid_build(&ring_grid, &rings);

	// Classify each ring as an outer edge or a hole by comparing how the fill
	// rule treats the space just inside and just outside of it. Rings that
	// don't change the fill are redundant.
	for (int32_t i = 0; i < rings.count; i++) {
		tri_ring_t* r = &rings[i];
		if (r->dropped) continue;

		int32_t orient  = r->area > 0 ? 1 : -1;
		int32_t winding = 0;
		int32_t depth   = 0;
		vec2    test    = r->inner;
		int32_t cell    = tri_grid_cell(&ring_grid, test);
		for (int32_t g = ring_grid.cell_start[cell]; g < ring_grid.cell_start[cell+1]; g++) {
			int32_t o = ring_grid.items[g];
			if (o == i || rings[o].dropped) continue;
			if (tri_ring_contains(points, &rings[o], test)) {
				winding += rings[o].area > 0 ? 1 : -1;
				depth   += 1;
			}
		}
		bool filled_out = even_odd ? (depth & 1) == 1     :  winding         != 0;
		bool filled_in  = even_odd ? ((depth+1) & 1) == 1 : (winding+orient) != 0;
		if      ( filled_in && !filled_out) r->is_hole = false;
		else if (!filled_in &&  filled_out) r->is_hole = true;
		else                                r->dropped = true;
	}

	// Holes belong to the smallest outer ring that contains them.
	for (int32_t i = 0; i < rings.count; i++) {
		tri_ring_t* r = &rings[i];
		if (r->dropped || !r->is_hole) continue;

		float   best_area = FLT_MAX;
		int32_t cell      = tri_grid_cell(&ring_grid, r->inner);
		for (int32_t g = ring_grid.cell_start[cell]; g < ring_grid.cell_start[cell+1]; g++) {
			int32_t o = ring_grid.items[g];
			if (rings[o].dropped || rings[o].is_hole) continue;
			float area = fabsf(rings[o].area);
			if (area < best_area && tri_ring_contains(points, &rings[o], r->inner)) {
				best_area = area;
				r->parent = o;
			}
		}
		if (r->parent < 0) r->dropped = true;
	}

	array_t<tri_node_t> nodes       = {};
	array_t<tri_ring_t> holes       = {};
	tri_grid_t          reflex_grid = {};
	for (int32_t i = 0; i < rings.count; i++) {
		tri_ring_t* outer = &rings[i];
		if (outer->dropped || outer->is_hole) continue;

		// Holes get bridged right to left so each ray cast can land on holes
		// already joined to the main loop.
		holes.clear();
		for (int32_t h = 0; h < rings.count; h++)
			if (!rings[h].dropped && rings[h].is_hole && rings[h].parent == i) holes.add(rings[h]);
		if (holes.count > 1) qsort(holes.data, holes.count, sizeof(tri_ring_t), tri_hole_compare);

		nodes.clear();
		int32_t head  = tri_add_loop(&nodes, outer->start, outer->count, outer->area < 0);
		int32_t count = outer->count;
		for (int32_t h = 0; h < holes.count; h++) {
			int32_t hole_head = tri_add_loop(&nodes, holes[h].start, holes[h].count, holes[h].area > 0);
			if (tri_bridge_hole(points, &nodes, head, hole_head))
				count += holes[h].count + 2;
		}
		tri_earclip(points, nodes.data, head, count, &reflex_grid, ref_indices);
	}

	ring_grid  .cell_start.free();
	ring_grid  .items     .free();
	reflex_grid.cell_start.free();
	reflex_grid.items     .free();
	holes.free();
	nodes.free();
	rings.free();
}

}
