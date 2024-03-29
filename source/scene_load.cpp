#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "common.hpp"
#include "obj_geometry.hpp"
#include "parse.hpp"
#include "scene.hpp"
#include "shapes.hpp"


using std::string;
using std::vector;


struct input_env {
  parse_env penv;

  scene *s;

  bool default_cam = true;

  bool default_mat = true;
  object_material material = { vec(0,0,0), vec(0,0,0), 
                                vec(0,0,0), 1, vec(0,0,0) };

  transform3f transform_ow;
};


static void add_scene_object(input_env *env, scene_object *obj) {
  if (env->default_mat)
    parse_warning(&env->penv, "Using default material for object");

  obj->material = env->material;

  if (obj->apply_affine(env->transform_ow))
    obj->transform_ow = trans3_identity();
  else
    obj->transform_ow = env->transform_ow;

  env->s->objects.push_back(obj);
};


static void exec_command(input_env *env, string line) {
  string cmd = parse_cmd(&env->penv, &line);

  if (cmd == "cam") {
    if (!env->transform_ow.identity)
      parse_warning(&env->penv, "Transformations do not currently "
                                "apply to cameras");
    env->s->camera.eye = parse_vec3f(&env->penv, &line);
    env->s->camera.lower_left = parse_vec3f(&env->penv, &line);
    env->s->camera.lower_right = parse_vec3f(&env->penv, &line);
    env->s->camera.upper_left = parse_vec3f(&env->penv, &line);
    env->s->camera.upper_right = parse_vec3f(&env->penv, &line);
    env->default_cam = false;

  } else if (cmd == "xfz") {
    env->transform_ow = trans3_identity();

  } else if (cmd == "xft") {
    vec3f t = parse_vec3f(&env->penv, &line);
    env->transform_ow *= trans3_translate(t);

  } else if (cmd == "xfs") {
    vec3f s = parse_vec3f(&env->penv, &line);
    if (s.x == 0 || s.y == 0 || s.z == 0)
      parse_error(&env->penv, "Scaling factors should be finite and nonzero");
    env->transform_ow *= trans3_scale(s.x, s.y, s.z);

  } else if (cmd == "xfr") {
    vec3f r = parse_vec3f(&env->penv, &line);
    rtfloat a = magnitude(r) * 3.14159265358979 / 180;
    r = normalize(r);
    env->transform_ow *= trans3_rotate(r, a);

  } else if (cmd == "lta") {
    light_source light;
    light.type = light_type::ambient;
    light.color = parse_vec3f(&env->penv, &line);
    env->s->lights.push_back(light);

  } else if (cmd == "ltd") {
    if (!env->transform_ow.identity)
      parse_warning(&env->penv, "Transformations do not currently apply "
                                "to lights");
    light_source light;
    light.type = light_type::directional;
    light.dir = normalize(parse_vec3f(&env->penv, &line));
    light.color = parse_vec3f(&env->penv, &line);
    env->s->lights.push_back(light);

  } else if (cmd == "ltp") {
    if (!env->transform_ow.identity)
      parse_warning(&env->penv, "Transformations do not currently apply "
                                "to lights");
    light_source light;
    light.type = light_type::point;
    light.pos = parse_vec3f(&env->penv, &line);
    light.color = parse_vec3f(&env->penv, &line);
    light.falloff = (int) parse_opt_int(&env->penv, &line, 0);
    if (light.falloff < 0 || light.falloff > 2) {
      parse_error(&env->penv, "Invalid falloff: %d", light.falloff);
      light.falloff = 0;
    }
    env->s->lights.push_back(light);
  
  } else if (cmd == "mat") {
    env->material.ambient = parse_vec3f(&env->penv, &line);
    env->material.diffuse = parse_vec3f(&env->penv, &line);
    env->material.specular = parse_vec3f(&env->penv, &line);
    env->material.specular_power = (rtfloat) parse_float(&env->penv, &line);
    env->material.reflective = parse_vec3f(&env->penv, &line);
    env->default_mat = false;

  } else if (cmd == "sph") {
    sphere_object *sphere = new sphere_object;
    sphere->center = parse_vec3f(&env->penv, &line);
    sphere->radius = (rtfloat) parse_float(&env->penv, &line);
    add_scene_object(env, sphere);
  
  } else if (cmd == "tri") {
    triangle_object *triangle = new triangle_object;
    triangle->vertices[0] = parse_vec3f(&env->penv, &line);
    triangle->vertices[1] = parse_vec3f(&env->penv, &line);
    triangle->vertices[2] = parse_vec3f(&env->penv, &line);
    triangle->default_normals();
    add_scene_object(env, triangle);

  } else if (cmd == "obj") {
    std::string filename = parse_string(&env->penv, &line);
    filename = env->penv.directory + filename;
    obj_geometry *obj = obj_read(filename.c_str());

    for (obj_triangle tri : obj->triangles) {
      triangle_object *triangle = new triangle_object;
      for (int i = 0; i < 3; i++) {
        size_t index = tri.vertices[i].vertex_index;
        triangle->vertices[i] = obj->vertices[index];
        triangle->normals[i] = tri.vertices[i].normal;
      }
      add_scene_object(env, triangle);
    }
    obj_destroy(obj);

  } else {
    parse_warning(&env->penv, "Unsupported command '%s'", cmd.c_str());
    return;
  }

  while (isspace(line[0])) line.erase(0, 1);
  if (line.size() > 0)
    parse_warning(&env->penv, "Extraneous arguments '%s'", line.c_str());
}


scene *scene_create(FILE *input, std::string filename) {
  input_env env;
  env.penv = parse_env_create(filename);
  env.transform_ow = trans3_identity();

  env.s = new scene;
  env.s->camera = { {0,0,0}, {-0.5,-0.5,-1}, {0.5,-0.5,-1}, 
                   {-0.5,0.5,-1}, {0.5,0.5,-1} };

  string line;
  while ((line = read_line(&env.penv, input)) != "")
    exec_command(&env, line);

  if (env.penv.error) {
    scene_destroy(env.s);
    return nullptr;
  }
  if (env.default_cam)
    fprintf(stderr, "Warning: Using default camera\n");

  return env.s;
}


void scene_destroy(scene *s) {
  if (s == nullptr) return;

  delete s->obj_structure;

  for (scene_object *obj : s->objects)
    delete obj;
  delete s;
}
