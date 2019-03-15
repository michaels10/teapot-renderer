#ifndef RENDER_H
#define RENDER_H
#include "camera.h"
#include "linalg.h"
#include "stdio.h"
#include "mesh.h"
#include <algorithm>
#include <cmath>
#include <iostream>
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
using std::sqrt;
using std::vector;

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
    Camera camera;
    int max_reflections;

    float render_ray(const Ray &ray, double multiplier, int reflection_count);
    void render(Canvas &canvas, Camera &camera);

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
#endif
