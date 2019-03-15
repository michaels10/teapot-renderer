#ifndef MESH_H
#define MESH_H 
#include "render.h"
#include "linalg.h"
#include <vector>
using namespace std;

// forward declarations
struct Canvas;
struct BoundingBox;
struct RaycastResult;

struct Triangle
{
  public:
    int v0, v1, v2, normal;
    Triangle() {}
    Triangle(int v0, int v1, int v2, int normal){
        this->v0 = v0;
        this->v1 = v1;
        this->v2 = v2;
        this->normal = normal;
    }
    BoundingBox get_bounds() const;
};

struct Mesh 
{
  public:
    vector<Vec3> verts;
    vector<Vec3> normals;
    vector<Triangle> tris;
    float ior = 1.5;
    float scattering = .95;
    float refraction = .00;

    Mesh(float *verts, size_t v_len, int *tris, size_t t_len, float scattering, float refraction, 
            float ior) {
        this->verts = vector<Vec3>();
        this->normals = vector<Vec3>();
        this->tris = vector<Triangle>();

        for (size_t i = 0; i < v_len * 3; i += 3) {
            Vec3 cur_vert = Vec3(verts[i], verts[i + 1], verts[i + 2]);
            this->verts.push_back(cur_vert);
        }

        for (size_t i = 0; i < t_len * 3; i += 3) {
            Triangle cur_tri = Triangle(tris[i], tris[i + 1], tris[i + 2], i);
            printf("%d, %d, %d\n", cur_tri.v0, cur_tri.v1, cur_tri.v2);
            this->tris.push_back(cur_tri);

            Vec3 v0 = verts[cur_tri.v0];
            Vec3 v1 = verts[cur_tri.v1];
            Vec3 v2 = verts[cur_tri.v2];
            Vec3 cur_normal = (v1 - v0) % (v2 - v0);
            this->normals.push_back(cur_normal);
        }
        this->scattering = scattering;
        this->refraction = refraction;
        this->ior = ior;
    }

    RaycastResult intersect(const Vec3 &origin, const Vec3 &ray);
    RaycastResult raycast(const Vec3 &origin, const Vec3 &ray, const Triangle &tri);
};
#endif
