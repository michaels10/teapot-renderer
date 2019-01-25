#ifndef RENDER_H
#define RENDER_H
#include "linalg.h"
#include "ray_queue.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <ostream>
#include <queue>
#include <vector>
#include "stdio.h"
using std::abs;
using std::cout;
using std::endl;
using std::max;
using std::min;
using std::queue;
using std::sqrt;
using std::swap;
using std::vector;
using std::ref;

// forward declarations
struct Triangle;
struct RayQueue;

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
    Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2) : v0(v0), v1(v1), v2(v2) {
        normal = ((v1 - v0) % (v2 - v0)).normalize();
    };
    Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &normal)
        : v0(v0), v1(v1), v2(v2), normal(normal){};

    Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &normal,
             float refraction_index, float scattering)
        : v0(v0), v1(v1), v2(v2), normal(normal), scattering(scattering),
          refraction_index(refraction_index){};
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
    Vec3 loc;
    Vec3 rotation; // YPR
    float focal_plane_width = 10;
    float focal_plane_height = 10;
    float focal_plane_distance = 10;
}

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

struct RayTask {
  public:
    Vec3 origin;
    Vec3 ray;
    int i, j;
    float multiplier;
    float refraction_index;
    int bounce_count;
};

RaycastResult raycast(const Vec3 &origin, const Vec3 &ray, const Triangle &tri);
void subrender(Canvas &canvas, const Scene &scene, int max_reflections, RayQueue &ray_queue);
void render(Canvas &canvas, const Scene &scene);
#endif
