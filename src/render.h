#ifndef RENDER_H
#define RENDER_H
#include "linalg.h"
#include "stdio.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <mutex>
#include <ostream>
#include <queue>
#include <string.h>
#include <thread>
#include <vector>
using std::abs;
using std::cout;
using std::endl;
using std::max;
using std::min;
using std::mutex;
using std::queue;
using std::ref;
using std::sqrt;
using std::swap;
using std::thread;
using std::vector;

// enum declarations
const int AUTO_LINEAR_EXPOSURE = 0;
const int MANUAL_LINEAR_EXPOSURE = 1;

// forward declarations
struct Triangle;
struct Canvas;

Triangle const operator-(const Triangle &tri, const Vec3 &vec);
Triangle const operator+(const Triangle &tri, const Vec3 &vec);

// Char array pulled from external source, should be the approximate 70
// ASCII chars with pixel density approximately scaling from 0.0-1.0
// Used for terminal output of scaled/normalized float arrays
const char *kConstBrightnessGradient =
    "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

// Enum used for term_out_image that selects for which output format is
// desired
// FLOAT for 0.0-1.0
// NUM for 0 - 70
// IMAGE for output with ASCII characters
enum kConstOutputSelector { FLOAT, NUM, IMAGE };

struct Triangle {
public:
  Vec3 v0, v1, v2, normal;
  float refraction_index = 1.5;
  float scattering = 0.1; // opacity
  /**
   * Create a triangle using CW winding order.
   **/
  Triangle() {}
  Triangle(const Triangle &other) {
    v0 = other.v0;
    v1 = other.v1;
    v2 = other.v2;
    normal = other.normal;
    refraction_index = other.refraction_index;
    scattering = other.scattering;
  }
  Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2)
      : v0(v0), v1(v1), v2(v2) {
    normal = ((v1 - v0) % (v2 - v1)).normalize();
  };
  Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &normal)
      : v0(v0), v1(v1), v2(v2) {
    this->normal = ((v1 - v0) % (v2 - v1)).normalize();
  };

  Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &normal,
           float refraction_index, float scattering)
      : v0(v0), v1(v1), v2(v2), refraction_index(refraction_index),
        scattering(scattering) {
    this->normal = ((v1 - v0) % (v2 - v1)).normalize();
  };
  void operator=(const Triangle &other) {
    v0 = other.v0;
    v1 = other.v1;
    v2 = other.v2;
    normal = other.normal;
    refraction_index = other.refraction_index;
    scattering = other.scattering;
  }
};

struct Light {
public:
  Vec3 loc;
  float intensity = 1.0;
};

struct Camera {
public:
  Vec3 loc = Vec3(0, 4, -10);
  Vec3 rotation; // = Vec3(-0.523599, 0, 0); // PYR, default is looking down Z+
  float focal_plane_width = 1;
  float focal_plane_height = 1;
  float focal_plane_distance = 100;
  int exposure_mode = AUTO_LINEAR_EXPOSURE;
  float max_exposure_energy = 1.0f;
  void expose(Canvas &canvas) const;
  int max_reflections = 256;
};

struct Scene {
public:
  vector<Triangle> geometry;
  vector<Light> lights;
};

struct RaycastResult {
public:
  Vec3 intersect;
  Triangle triangle;
  float distance = 999999;
  bool hit = false;
  RaycastResult(Vec3 intersect, float t, Triangle tri)
      : intersect(intersect), distance(t) {
    triangle = tri;
    hit = true;
  };
  RaycastResult(bool hit) { this->hit = hit; };
  RaycastResult(){};
};

struct Canvas {
public:
  int width, height;
  float *buffer;

  Canvas(int rows, int cols) : width(cols), height(rows) {
    buffer = new float[width * height];
    memset(buffer, 0.0f, width * height * sizeof(float));
  }
  ~Canvas() { delete[] buffer; }
  float *operator[](int row) { return &buffer[row * width]; }
  // Takes in an image array (float 0.0-1.0), with specified dimensions and
  // returns a pointer to the resized (ensmallened) float array
  void ensmallen(const int &out_x, const int &out_y);
  // Visual debugger that takes in the pointer to the float array and
  // pretty-prints it to the terminal
  // Clunky seelctor may be reworked? Carries an enum on its own and blah
  void term_out_image(const char *title, const kConstOutputSelector &selector);
};

struct Ray {
public:
  Vec3 origin;
  Vec3 ray;
  float refraction_index = 1;
};

void render_ray(Canvas &canvas, const Scene &scene, const Ray &ray, int i,
                int j, float multiplier, int reflection_count,
                int max_reflections);
void subrender(Canvas &canvas, const Scene &scene, const Camera &camera,
               queue<int> &block_queue, mutex &queue_lock);
Ray get_initial_ray(const Canvas &canvas, const Camera &camera, int ray_id);
void render(Canvas &canvas, const Scene &scene, const Camera &camera);
#endif
