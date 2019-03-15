#ifndef RENDER_H
#define RENDER_H
#include "linalg.h"
#include "mesh.h"
#include "stdio.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <ostream>
#include <queue>
#include <string.h>
#include <thread>
#include <vector>
using std::abs;
using std::endl;
using std::max;
using std::min;
using std::sqrt;
using std::swap;
using std::vector;

// forward declarations
struct Canvas;
struct BoundingBox;
struct Mesh;
struct RaycastResult;
struct Ray;

const int AUTO_LINEAR_EXPOSURE = 0;
const int MANUAL_LINEAR_EXPOSURE = 1;

struct Light {
  public:
    Vec3 loc;
    float intensity = 1.0;
};

struct Camera {
  public:
    Vec3 loc = Vec3(0, 4, -5);
    Vec3 rotation; // = Vec3(-0.523599, 0, 0); // PYR, default is looking down Z+

    // Camera settings
    float focal_plane_distance = 1;
    float focal_plane_width = 2;
    float focal_plane_height = 2;
    int exposure_mode = AUTO_LINEAR_EXPOSURE;
    float max_exposure_energy = 55.0f;

    Camera(float focal_distance, float width, float height, float max_exposure) {
        this->focal_plane_distance = focal_distance;
        this->focal_plane_width = width;
        this->focal_plane_height = height;
        this->max_exposure_energy = max_exposure;

        exposure_mode = MANUAL_LINEAR_EXPOSURE;
    }

    Camera(float focal_distance, float width, float height) {
        this->focal_plane_distance = focal_distance;
        this->focal_plane_width = width;
        this->focal_plane_height = height;

        exposure_mode = AUTO_LINEAR_EXPOSURE;
    }

    Camera() {}

    void expose(Canvas &canvas);
    Ray get_initial_ray(int i, int j, const Canvas &canvas) const;
};

struct Scene {
  public:
    Scene(){};
    vector<Mesh> geometry;
    vector<Light> lights;
    Camera camera;
    int max_reflections = 8;

    float render_ray(const Ray &ray, float multiplier, int reflection_count);
    void render(Canvas &canvas);

    RaycastResult all_intersect(const Vec3 &origin, const Vec3 &ray);
    bool any_intersect(const Vec3 &origin, const Vec3 &ray);

    float local_illuminate(const RaycastResult &hit);
    float fresnel(const Ray &incident, const RaycastResult &intersect);
    Ray refract(const Ray &incident, const RaycastResult &intersect);
    Ray reflect(const Ray &incident, const RaycastResult &intersect);
};

struct Ray {
  public:
    Vec3 origin;
    Vec3 ray;
    float ior = 1;
};

struct Canvas {
  public:
    size_t width, height;
    float *buffer;
    Canvas() {}
    Canvas(size_t width, size_t height) {
        this->width = width;
        this->height = height;
        buffer = new float[width * height];
        memset(buffer, 0.0f, width * height * sizeof(float));
    }

    ~Canvas() { delete[] buffer; }

    float *operator[](int row) { return &buffer[height * row]; }
};

struct RaycastResult {
  public:
    Vec3 intersect;
    Vec3 normal;
    float distance = 999999;
    float scattering = 1.0;
    float ior = 1.3;
    bool hit = false;

    RaycastResult(Vec3 intersect, Vec3 normal, float scattering, float ior, float t) {

        this->normal = Vec3(normal);
        this->intersect = Vec3(intersect);
        this->scattering = scattering;
        this->ior = ior;
        this->distance = t;
        hit = true;
    };
    RaycastResult(bool hit) { this->hit = hit; };
    RaycastResult(){};
};

#endif
