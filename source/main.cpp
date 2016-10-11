#include <cstdio>
#include <string>

#include "common.hpp"
#include "image.hpp"
#include "scene.hpp"
#include "shapes.hpp"


static FILE *in_file = stdin;
static FILE *out_file = stdout;
static size_t img_width = 700, img_height = 700;


static void read_arguments(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "--size" || arg == "-s") {
      if (i + 2 >= argc) {
        fprintf(stderr, "Error: Expected two numbers after --size flag\n");
        exit(1);
      }
      std::string w = argv[++i];
      std::string h = argv[++i];

      const char *endptr = w.c_str();
      int width = (int) strtol(w.c_str(), (char **) &endptr, 10);
      if (endptr != w.c_str() + w.size() || width < 0) {
        fprintf(stderr, "Error: Invalid first argument to --size\n");
        exit(1);
      }

      endptr = h.c_str();
      int height = (int) strtol(h.c_str(), (char **) &endptr, 10);
      if (endptr != h.c_str() + h.size() || height < 0) {
        fprintf(stderr, "Error: Invalid second argument to --size\n");
        exit(1);
      }

      img_width = (size_t) width;
      img_height = (size_t) height;
    

    } else if (arg == "--output" || arg == "-o") {
      if (i + 1 >= argc) {
        fprintf(stderr, "Error: Expected filename after --output flag\n");
        exit(1);
      }
      std::string filename = argv[++i];

      out_file = fopen(filename.c_str(), "w");
      if (out_file == nullptr) {
        fprintf(stderr, "Error: Failed to open %s\n", filename.c_str());
        exit(1);
      }


    } else if (arg[0] == '-') {
      fprintf(stderr, "Error: Unknown flag: %s\n", arg.c_str());
      exit(1);


    } else {
      in_file = fopen(arg.c_str(), "r");
      if (in_file == nullptr) {
        fprintf(stderr, "Error: Failed to open %s\n", arg.c_str());
        exit(1);
      }
    }
  }
}


int main(int argc, char *argv[]) {
  read_arguments(argc, argv);

  scene *s = scene_create(in_file);
  if (in_file != stdin)
    fclose(in_file);
  if (s == nullptr) exit(1);

  image_output_stream write_pixel = ppm_stream(out_file, img_width, img_height);
  scene_render(s, img_width, img_height, write_pixel);

  if (out_file != stdout)
    fclose(out_file);

  scene_destroy(s);
  return 0;
}
