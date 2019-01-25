#ifndef RENDER_H
#define RENDER_H
#include <algorithm>
#include <cmath>
#include <iostream>
#include <ostream>
#include <queue>
#include <vector>
using std::abs;
using std::cout;
using std::endl;
using std::max;
using std::min;
using std::queue;
using std::sqrt;
using std::swap;
using std::vector;

// forward declarations
struct Vec3;
struct Triangle;
Vec3 operator+(const Vec3 &v0, const Vec3 &v1);
Vec3 const operator-(const Vec3 &v0, const Vec3 &v1);
Vec3 const operator-(const Vec3 &v0);
Vec3 const operator*(const Vec3 &v0, const Vec3 &v1);
Vec3 const operator*(const Vec3 &v0, float s);
Vec3 const operator*(float s, const Vec3 &v0);
Vec3 const operator/(const Vec3 &v0, float s);
Vec3 const operator/(float s, const Vec3 &v0);
Vec3 const operator%(const Vec3 &v0, const Vec3 &v1);
Triangle const operator-(const Triangle &tri, const Vec3 &vec);
Triangle const operator+(const Triangle &tri, const Vec3 &vec);
float const operator^(const Vec3 &v0, const Vec3 &v1);

struct Vec3 {
  public:
    float x, y, z;
    bool valid = true;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z){};
    Vec3 normalize() const;
    float magnitude() const;
    float sum() const;
    float dot(const Vec3 &other) const;
    Vec3 intersect(const Triangle &tri) const;

    Vec3(const Vec3 &other);
    void operator=(const Vec3 &other);

    bool operator==(const Vec3 &other);
};

struct Triangle {
  public:
    Vec3 v0, v1, v2, normal;
    float refraction_index = 1.5;
    float scattering = 0.1; //opacity
    /**
     * Create a triangle using CW winding order.
     **/
    Triangle() {}
    Triangle(const Triangle &other) {
        v0 = other.v0;
        v1 = other.v1;
        v2 = other.v2;
        normal = other.normal;
    }
    Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2) : v0(v0), v1(v1), v2(v2) { normal = ((v1 - v0) % (v2 - v0)).normalize(); };
    Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &normal) : v0(v0), v1(v1), v2(v2), normal(normal){};
    void operator=(const Triangle &other) {
        v0 = other.v0;
        v1 = other.v1;
        v2 = other.v2;
        normal = other.normal;
    }
};

struct Light {
  public:
    Vec3 loc;
    float intensity = 1.0;
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

struct RayTask {
  public:
    Vec3 origin;
    Vec3 ray;
    int i, j;
    float multiplier;
    float refraction_index;
    int bounce_count;
};

struct Mat3 {
  public:
    float data[3][3];
    Mat3(const Vec3 &c0, const Vec3 &c1, const Vec3 &c2) {
        data[0][0] = c0.x;
        data[1][0] = c0.y;
        data[2][0] = c0.z;
        data[0][1] = c1.x;
        data[1][1] = c1.y;
        data[2][1] = c1.z;
        data[0][2] = c2.x;
        data[1][2] = c2.y;
        data[2][2] = c2.z;
    }
    Vec3 solve(const Vec3 &rhs);
    float det();
};

RaycastResult raycast(const Vec3 &origin, const Vec3 &ray, const Triangle &tri);
void subrender(Canvas &canvas, vector<RayTask> tasks);
void render(Canvas &canvas, const Scene &scene);
#endif