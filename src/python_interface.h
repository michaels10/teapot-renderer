#ifndef PYTHON_INTERF_H
#define PYTHON_INTERF_H
#include "render.h"

typedef struct PyVec3 {
  float x, y, z;
} PyVec3;

typedef struct PyTriangle {
  PyVec3 v0;
  PyVec3 v1;
  PyVec3 v2;
  PyVec3 normal;
  float scattering;
  float refraction_index;
} PyTriangle;

typedef struct PyCanvas {
  int width;
  int height;
  float *canvas;
  Canvas* cpp_canvas;
} PyCanvas;

typedef struct PyScene {
  Scene* scene;
} PyScene;

typedef struct PyLight {
  PyVec3 loc;
  float intensity;
} PyLight;

extern "C" void add_triangle(PyTriangle *tri, PyScene *scene);
extern "C" void add_light(PyLight *pylight, PyScene *scene);
extern "C" void __init_scene(PyScene *scene);
extern "C" void __init_canvas(PyCanvas *canvas, int width, int height);
extern "C" void render(PyScene* scene, PyCanvas* canvas);

#endif