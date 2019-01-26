#ifndef RENDER_H
#define RENDER_H
#include "linalg.h"
#include "stdio.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <ostream>
#include <queue>
#include <thread>
#include <vector>
#include <mutex>
using std::abs;
using std::cout;
using std::endl;
using std::max;
using std::min;
using std::queue;
using std::ref;
using std::sqrt;
using std::swap;
using std::thread;
using std::vector;
using std::mutex;

// forward declarations
struct Triangle;

Triangle const operator-(const Triangle &tri, const Vec3 &vec);
Triangle const operator+(const Triangle &tri, const Vec3 &vec);

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
    Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2) : v0(v0), v1(v1), v2(v2) { normal = ((v1 - v0) % (v2 - v0)).normalize(); };
    Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &normal) : v0(v0), v1(v1), v2(v2), normal(normal){};

    Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &normal, float refraction_index, float scattering)
        : v0(v0), v1(v1), v2(v2), normal(normal), refraction_index(refraction_index), scattering(scattering){};
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
    Vec3 rotation;// = Vec3(-0.523599, 0, 0); // PYR, default is looking down Z+
    float focal_plane_width = 1;
    float focal_plane_height = 1;
    float focal_plane_distance = 1000;
    int max_reflections = 128;
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
    RaycastResult(Vec3 intersect, float t, Triangle tri) : intersect(intersect), distance(t) {
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

    Canvas(int rows, int cols) : width(cols), height(rows) { buffer = new float[width * height]; }
    ~Canvas() { delete[] buffer; }
    float *operator[](int row) { return &buffer[row * width]; }
};

struct Ray {
  public:
    Vec3 origin;
    Vec3 ray;
    float refraction_index = 1;
};

void render_ray(Canvas &canvas, const Scene &scene, const Ray &ray, int i, int j, float multiplier, int reflection_count, int max_reflections);
void subrender(Canvas &canvas, const Scene &scene, const Camera &camera, queue<int> &block_queue, mutex &queue_lock);
Ray get_initial_ray(const Canvas &canvas, const Camera &camera, int ray_id);
void render(Canvas &canvas, const Scene &scene, const Camera &camera);
#endif
