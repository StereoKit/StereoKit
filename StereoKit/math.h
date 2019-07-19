#pragma once

struct vec2 {
	float x, y;
};
struct vec3 {
	float x, y, z;
};
struct quat {
	float i, j, k, a;
};

static inline vec2 operator*(const vec2 &a, const float b) { return { a.x * b, a.y * b }; }
static inline vec2 operator/(const vec2 &a, const float b) { return { a.x / b, a.y / b }; }
static inline vec2 operator+(const vec2 &a, const vec2 &b) { return { a.x + b.x, a.y + b.y }; }
static inline vec2 operator-(const vec2 &a, const vec2 &b) { return { a.x - b.x, a.y - b.y }; }
static inline vec2 operator*(const vec2 &a, const vec2 &b) { return { a.x * b.x, a.y * b.y }; }
static inline vec2 operator/(const vec2 &a, const vec2 &b) { return { a.x / b.x, a.y / b.y }; }
static inline vec2& operator+=(vec2& a, const vec2& b)     { a.x += b.x; a.y += b.y; return a; }
static inline vec2& operator-=(vec2& a, const vec2& b)     { a.x -= b.x; a.y -= b.y; return a; }
static inline vec2& operator*=(vec2& a, const float b)     { a.x *= b; a.y *= b; return a; }
static inline vec2& operator/=(vec2& a, const float b)     { a.x /= b; a.y /= b; return a; }

static inline vec3 operator*(const vec3 &a, const float b) { return { a.x * b, a.y * b, a.z * b }; }
static inline vec3 operator/(const vec3 &a, const float b) { return { a.x / b, a.y / b, a.z / b }; }
static inline vec3 operator+(const vec3 &a, const vec3 &b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline vec3 operator-(const vec3 &a, const vec3 &b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline vec3 operator*(const vec3 &a, const vec3 &b) { return { a.x * b.x, a.y * b.y, a.z * b.z }; }
static inline vec3 operator/(const vec3 &a, const vec3 &b) { return { a.x / b.x, a.y / b.y, a.z / b.z }; }
static inline vec3& operator+=(vec3& a, const vec3& b)     { a.x += b.x; a.y += b.y; a.z += b.z; return a; }
static inline vec3& operator-=(vec3& a, const vec3& b)     { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a; }
static inline vec3& operator*=(vec3& a, const float b)     { a.x *= b; a.y *= b; a.z *= b; return a; }
static inline vec3& operator/=(vec3& a, const float b)     { a.x /= b; a.y /= b; a.z /= b; return a; }

#define deg2rad 0.01745329252
#define rad2deg 57.295779513