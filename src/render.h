#ifndef RENDER_H
#define RENDER_H
#include "linalg.h"
#include "stdio.h"
//#include "octree.h"
#include "mesh.h"
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
struct Canvas;
struct BoundingBox;

struct Light {
  public:
    Vec3 loc;
    float intensity = 1.0;
};

struct Scene {
  public:
    vector<Mesh> geometry;
    vector<Light> lights;
};

struct Ray {
  public:
    Vec3 origin;
    Vec3 ray;
    float ior = 1;
};

struct RaycastResult {
  public:
    Vec3 intersect;
    Vec3 normal;
    float distance = 999999;
    float scattering = 1.0;
    float refraction = 0.0;
    float ior = 1.3;
    bool hit = false;

    RaycastResult(Vec3 intersect, Vec3 normal, float scattering, float refraction, 
            float ior, float t) {

        this->normal = Vec3(normal);
        this->intersect = Vec3(intersect);
        this->scattering = scattering;
        this->refraction = refraction;
        this->ior = ior;
        this->distance = t;
        hit = true;
    };
    RaycastResult(bool hit) { this->hit = hit; };
    RaycastResult() {};
};

struct Canvas {
  public:
    size_t width, height; 
    float * buffer;
    Canvas () {}
    Canvas(size_t width, size_t height) {
        this->width = width;
        this->height = height;
        buffer = new float[width * height];
        memset(buffer, 0.0f, width * height * sizeof(float));
    }

    ~Canvas() { 
        delete[] buffer; 
    }

    float *operator[](int row) { 
        return &buffer[height * row]; 
    }
};

struct Camera {
  public:
    Vec3 loc = Vec3(0, 4, -10);
    Vec3 rotation; // = Vec3(-0.523599, 0, 0); // PYR, default is looking down Z+

    // Camera settings
    float focal_plane_distance = 1;
    float focal_plane_width = 4;
    float focal_plane_height = 4;
    int exposure_mode = AUTO_LINEAR_EXPOSURE;
    float max_exposure_energy = 55.0f;

    // Render settings
    float multiplier = 1;
    int reflection_count = 1, max_reflections = 8;

    // canvas settings
    Canvas canvas;

    Camera() {};
    Camera(float focal_distance, float width, float height, float max_exposure, int res_x, int res_y) {
        this->focal_plane_distance = focal_distance;
        this->focal_plane_width = width;
        this->focal_plane_height = height;
        this->max_exposure_energy = max_exposure;
        canvas = Canvas(res_x, res_y);

        exposure_mode = MANUAL_LINEAR_EXPOSURE;
    }

    Camera(float focal_distance, float width, float height, int res_x, int res_y) {
        this->focal_plane_distance = focal_distance;
        this->focal_plane_width = width;
        this->focal_plane_height = height;

        exposure_mode = AUTO_LINEAR_EXPOSURE;
    }

    void expose();
    Ray get_initial_ray(int i, int j) const;
    void render_ray(const Scene &scene, const Ray &ray, int i, int j, 
        double multiplier, int reflection_count);
};


#endif
