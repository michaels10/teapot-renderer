#ifndef PYTHON_INTERF_H
#define PYTHON_INTERF_H
#include "scene.h"
#include "linalg.h"

typedef struct PyVec3 {
  double x, y, z;
} PyVec3;

typedef struct PyLight {
  PyVec3 loc;
  float intensity;
} PyLight;

typedef struct PyCanvas {
  int width;
  int height;
  float *canvas;
  void* cpp_canvas;
} PyCanvas;

typedef struct PyScene {
  void* scene;
} PyScene;

extern "C" void add_mesh(PyScene * scene, float *vert, size_t vert_len,
		int *tris, size_t tris_len, float scttrng, float refraction, float ior);
extern "C" void add_light(PyScene *scene, PyLight *pylight);
extern "C" void __init_scene(PyScene *scene);
extern "C" void __init_canvas(PyCanvas *canvas, int width, int height);
extern "C" void render(PyScene* scene, int res_x, int res_y);

#endif
