#ifndef PYTHON_INTERF_H
#define PYTHON_INTERF_H

typedef struct PyVec3 {
  float x, y, z;
} PyVec3;

typedef struct PyTriangle {
  PyVec3 v0;
  PyVec3 v1;
  PyVec3 v2;
  PyVec3 normal;
} PyTriangle;

typedef struct PyCanvas {
  int width;
  int height;
  float *canvas;
} PyCanvas;

typedef struct PyLight {
  PyVec3 loc;
  float intensity;
} PyLight;

#endif