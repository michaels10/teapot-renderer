#ifndef VECTOR_H
#define VECTOR_H
#include <cmath>

const float EPS = 0.00001;

struct Vec3;

Vec3 const operator*(const Vec3 &v, float s);
Vec3 const operator*(float s, const Vec3 &v);
Vec3 const operator/(const Vec3 &v, float s);

struct Vec3 {
  public:
    float x, y, z;
    bool valid = true;

    Vec3(const Vec3 &other);
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z){};

    // No vector operators
    Vec3 operator-() const;

    // Single-vector operators
    void operator=(const Vec3 &other);
    bool operator==(const Vec3 &other);

    // Two-vector operations.
    Vec3 operator*(const Vec3 &v) const;
    Vec3 operator-(const Vec3 &v) const;
    Vec3 operator%(const Vec3 &v) const;
    Vec3 operator+(const Vec3 &v) const;

    // Helper functions
    Vec3 normalize() const;
    float magnitude() const;
    float sum() const;
    float dot(const Vec3 &v) const;
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


#endif
