#include <cmath>

#include "common.hpp"
#include "scene.hpp"
#include "shapes.hpp"


ray_intersection sphere_object::ray_test(ray3f ray) {
  vec3f p = ray.start - center;
  vec3f d = ray.dir;

  rtfloat a = dot(d, d);
  rtfloat b = 2 * dot(d, p);
  rtfloat c = dot(p, p) - radius * radius;

  rtfloat disc = b*b - 4*a*c;
  if (disc < 0)
    return no_intersection(this);

  rtfloat sd = sqrt(disc);
  rtfloat t1 = (-b + sd)/(2*a);
  rtfloat t2 = (-b - sd)/(2*a);
  if (t1 < 0) t1 = rtfloat_inf;
  if (t2 < 0) t2 = rtfloat_inf;

  rtfloat dist = std::min(t1, t2);
  if (dist < rtfloat_inf) {
    vec3f point = ray.start + dist * ray.dir;
    vec3f normal = normalize(point - center);
    return intersection(this, dist, normal);
  }

  return no_intersection(this);
}


aa_box3f sphere_object::bounding_box() {
  vec3f r = { radius, radius, radius };
  return { center - r, center + r };
}



void triangle_object::default_normals() {
  vec3f n = normalize(cross(vertices[0] - vertices[2], 
                            vertices[1] - vertices[2]));
  normals[0] = normals[1] = normals[2] = n;
}


ray_intersection triangle_object::ray_test(ray3f ray) {
  vec3f v1 = vertices[0], v2 = vertices[1], v3 = vertices[2];
  vec3f w1 = v1 - v3, w2 = v2 - v3;
  vec3f n = normalize(cross(w1, w2));

  rtfloat s = dot(n, v1 - ray.start) / dot(n, ray.dir);
  if (s == rtfloat_inf || s < 0) return no_intersection(this);

  vec3f rp = (ray.start + s * ray.dir) - v3;

  rtfloat t1 = rtfloat_inf, t2 = rtfloat_inf;
  rtfloat det;
  static const rtfloat tolerance = 0.001;
  if (std::abs(det = w1.x*w2.y - w2.x*w1.y) >= tolerance) {
    t1 = (w2.y*rp.x - w2.x*rp.y) / det;
    t2 = (w1.x*rp.y - w1.y*rp.x) / det;
  } else if (std::abs(det = w1.x*w2.z - w2.x*w1.z) >= tolerance) {
    t1 = (w2.z*rp.x - w2.x*rp.z) / det;
    t2 = (w1.x*rp.z - w1.z*rp.x) / det;
  } else if (std::abs(det = w1.y*w2.z - w2.y*w1.z) >= tolerance) {
    t1 = (w2.z*rp.y - w2.y*rp.z) / det;
    t2 = (w1.y*rp.z - w1.z*rp.y) / det;
  } else {
    // Triangle vertices are colinear
    return no_intersection(this);
  }

  if (t1 < 0 || t1 > 1 || t2 < 0 || t2 > 1 || t1+t2 > 1)
    return no_intersection(this);

  vec3f normal = t1*normals[0] + t2*normals[1] + (1-t1-t2)*normals[2];
  normal = normalize(normal);
  return intersection(this, s, normal);
}


static inline rtfloat min3(rtfloat a, rtfloat b, rtfloat c) {
  return std::min(a, std::min(b, c));
}

static inline rtfloat max3(rtfloat a, rtfloat b, rtfloat c) {
  return std::max(a, std::max(b, c));
}

aa_box3f triangle_object::bounding_box() {
  aa_box3f box;
  for (int k = 0; k < 3; k++) {
    box.low_v.data[k] = min3(vertices[0].data[k], vertices[1].data[k],
                              vertices[2].data[k]);
    box.high_v.data[k] = max3(vertices[0].data[k], vertices[1].data[k],
                              vertices[2].data[k]);
  }
  return box;
}


bool triangle_object::apply_affine(const transform3f &trans) {
  for (int i = 0; i < 3; i++)
    vertices[i] = project(trans * hpoint(vertices[i]));
  for (int i = 0; i < 3; i++)
    normals[i] = trans_normal(trans, normals[i]);
  return true;
}
