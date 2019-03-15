#ifndef RENDER_H
#define RENDER_H
#include "linalg.h"
#include "stdio.h"
#include "octree.h"
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

struct Triangle {
    public:
        int v0, v1, v2, normal;
        Triangle() {}
        Triangle(int v0, int v1, int v2, int normal): v0(v0), v1(v1), v2(v2), normal(normal) {}

    	BoundingBox get_bounds() const;
};

struct Mesh {
  public:
    vector<Vec3> verts;
    vector<Vec3> normals;
    vector<Triangle> tris;
    float ior = 1.5;
    float scattering = .95;

    Mesh(float * verts, size_t v_len, int * tris, size_t t_len, float scattering, float ior) {
	this->verts = vector<Vec3>();
	this->normals = vector<Vec3>();
	this->tris = vector<RefTriangle>();

	for (size_t i = 0; i < v_len*3; i += 3) {
	    Vec3 cur_vert = Vec3(verts[i], verts[i+1], verts[i+2]);
	    this->verts.push_back(cur_vert);
	}
	for (size_t i = 0; i < t_len*3; i += 3) {
	    RefTriangle cur_tri = RefTriangle(tris[i], tris[i+1], tris[i+2], i);
	    printf("%d, %d, %d", cur_tri.v0, cur_tri.v1, cur_tri.v2);
	    this->tris.push_back(cur_tri);

	    Vec3 v0 = verts[cur_tri.v0];
	    Vec3 v1 = verts[cur_tri.v1];
	    Vec3 v2 = verts[cur_tri.v2];
            Vec3 cur_normal = (v1 - v0) % (v2 - v0);
	    this->normals.push_back(cur_normal);
	}
	this->scattering = scattering;
	this->ior = ior;
    }

    Triangle get_triangle(Triangle tri) const{
        return Triangle();
    } 

};

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

struct Camera {
  public:
    Vec3 loc = Vec3(0, 4, -10);
    Vec3 rotation; // = Vec3(-0.523599, 0, 0); // PYR, default is looking down Z+

    int width, height;
    float *buffer;

    float focal_plane_distance = 1;
    float focal_plane_width = 4;
    float focal_plane_height = 4;
    int exposure_mode = AUTO_LINEAR_EXPOSURE;
    float max_exposure_energy = 55.0f;
    float multiplier;
    int reflection_count = , max_reflections;

    void expose(Canvas &canvas) const;

    Camera(){};
    Camera(float focal_distance, float width, float height, float max_exposure)
        : focal_plane_distance(focal_distance), focal_plane_width(width),
          focal_plane_height(height), max_exposure_energy(max_exposure) {
        exposure_mode = MANUAL_LINEAR_EXPOSURE;
    }
    Camera(float focal_distance, float width, float height)
        : focal_plane_distance(focal_distance), focal_plane_width(width),
          focal_plane_height(height) {
        exposure_mode = AUTO_LINEAR_EXPOSURE;
    }
    int max_reflections = 8;
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

    Canvas(int rows, int cols) : width(cols), height(rows) {
        buffer = new float[width * height];
        memset(buffer, 0.0f, width * height * sizeof(float));
    }
    ~Canvas() { delete[] buffer; }
    float *operator[](int row) { return &buffer[row * width]; }
};

struct Ray {
  public:
    Vec3 origin;
    Vec3 ray;
    float ior = 1;
};

void render_ray(Canvas &canvas, const Scene &scene, const Ray &ray, int i, int j, float multiplier,
                int reflection_count, int max_reflections);

void subrender(Canvas &canvas, const Scene &scene, const Camera &camera, queue<int> &block_queue,
               mutex &queue_lock);

Ray get_initial_ray(const Canvas &canvas, const Camera &camera, int ray_id);

void render(Canvas &canvas, const Scene &scene, const Camera &camera);
#endif
