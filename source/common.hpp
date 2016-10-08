#ifndef _RAYTRACER_COMMON_HPP
#define _RAYTRACER_COMMON_HPP


#include <cstdint>
#include <limits>
#include <string>
#include <vector>



typedef double rtfloat;

static const rtfloat rtfloat_inf = std::numeric_limits<rtfloat>().infinity();


/* Helpful utilities and simple math functions */

template <typename... T>
std::string stringf(const char *format, T... args);

template <typename T>
T clamp(T x, T lo, T hi);



/* Vector in 3-dim rtfloat space */

struct vec3f {
  union { rtfloat x; rtfloat r; };
  union { rtfloat y; rtfloat g; };
  union { rtfloat z; rtfloat b; };
};

typedef vec3f color3f;

vec3f vec(rtfloat x, rtfloat y, rtfloat z);
color3f color(rtfloat r, rtfloat g, rtfloat b);

std::string str(vec3f);

rtfloat magnitude(vec3f);
vec3f normalize(vec3f);
rtfloat dot(vec3f, vec3f);

vec3f operator+(vec3f, vec3f);
vec3f operator-(vec3f, vec3f);
vec3f operator-(vec3f);

vec3f operator*(vec3f, rtfloat);
vec3f operator*(rtfloat, vec3f);
vec3f operator/(vec3f, rtfloat);

vec3f operator*(vec3f, vec3f); // Hadamard product



/* Vector in 4-dim rtfloat space */

struct vec4f {
  rtfloat x, y, z, w;
};

typedef vec4f hvec3f;

vec4f vec(rtfloat x, rtfloat y, rtfloat z, rtfloat w);
vec4f vec(vec3f xyz, rtfloat w);
hvec3f hvec(vec3f v);
hvec3f hvec(rtfloat x, rtfloat y, rtfloat z);
hvec3f hpoint(vec3f p);
hvec3f hpoint(rtfloat x, rtfloat y, rtfloat z);

std::string str(vec4f);
std::string strp(vec4f);

vec3f project(vec4f);



/* Ray in homogeneous coordinates */

struct ray3f {
  vec3f start;
  vec3f dir;
};

ray3f ray(vec3f start, vec3f dir);

std::string str(ray3f);



/* 4x4 matrix over rtfloat */

struct matrix4f {
  rtfloat data[16];

  rtfloat &operator()(int row, int col);  
};

matrix4f mat4(std::vector<rtfloat> entries);

std::string str(matrix4f &);

matrix4f operator*(matrix4f &, matrix4f &);
vec4f operator*(matrix4f &, vec4f);
ray3f operator*(matrix4f &, ray3f);

matrix4f mat4_identity();
matrix4f mat4_zero();
matrix4f mat4_hzero();
matrix4f mat4_scale(rtfloat x, rtfloat y, rtfloat z, rtfloat w);
matrix4f mat4_hscale(rtfloat x, rtfloat y, rtfloat z);
matrix4f mat4_hrotate_x(rtfloat a);
matrix4f mat4_hrotate_y(rtfloat a);
matrix4f mat4_hrotate_z(rtfloat a);
matrix4f mat4_htranslate(vec3f v);



#include "utils.inl"
#include "vector.inl"
#include "matrix.inl"


#endif
