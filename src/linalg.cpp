#include "linalg.h"

Vec3::Vec3(const Vec3 &other) {
    x = other.x;
    y = other.y;
    z = other.z;
}

void Vec3::operator=(const Vec3 &other) {
    x = other.x;
    y = other.y;
    z = other.z;
}

bool Vec3::operator==(const Vec3 &other) { 
    return x == other.x && y == other.y && z == other.z; 
}

// Invert the vector
Vec3 Vec3::operator-() const{ 
    return Vec3(-x, -y, -z); 
}

// Element-wise multiplication
Vec3 Vec3::operator*(const Vec3 &v) const {
    return Vec3(x * v.x, y * v.y, z * v.z);
}

// Element-wise addition
Vec3 Vec3::operator+(const Vec3 &v) const {
    return Vec3(x + v.x, y + v.y, z + v.z);
}

// Vector subtraction
Vec3 Vec3::operator-(const Vec3 &v) const{
    return (*this) + (-v);
}

// Vector cross-product
Vec3 Vec3::operator%(const Vec3 &v) const{
    Vec3 v_out;
    v_out.x = y * v.z - z * v.y;
    v_out.y = z * v.x - x * v.z;
    v_out.z = x * v.y - y * v.x;
    return v_out;
}

float Vec3::sum() const { 
    return x + y + z; 
}

float Vec3::dot(const Vec3 &v) const { 
    return x * v.x +  y * v.y + z * v.z; 
}

float Vec3::magnitude() const { 
    return sqrt(x*x + y*y + z*z); 
}

// Normalize
Vec3 Vec3::normalize() const {
    float mag = magnitude();
    return Vec3(x/mag, y/mag, z/mag);
}

/************************ Externally defined vector operators ******************/
Vec3 const operator*(const Vec3 &v, float s) {
    return Vec3(v.x * s, v.y * s, v.z * s);
}

Vec3 const operator*(float s, const Vec3 &v) { 
    return v * s; 
}

Vec3 const operator/(const Vec3 &v, float s) {
    return Vec3(v.x/s, v.y/s, v.z/s);
}

/*********************** Matrix operations *************************************/

float Mat3::det() {
    float det = data[0][0] * (data[1][1] * data[2][2] - data[1][2] * data[2][1]);
    det -= data[0][1] * (data[1][0] * data[2][2] - data[1][2] * data[2][0]);
    det -= data[0][2] * (data[1][0] * data[2][1] - data[1][1] * data[2][0]);
    return det;
}

Vec3 Mat3::solve(const Vec3 &b) {
    double determinant = det();
    if ((-EPS < determinant && determinant < EPS) || data[0][0] == 0) {
        return Vec3(NAN, NAN, NAN);
    }

    double r1Rescale = data[1][0] / data[0][0];
    double b11 = data[1][1] - data[0][1] * r1Rescale;
    double b12 = data[1][2] - data[0][2] * r1Rescale;
    double c1 = b.y - b.x * r1Rescale;

    double r2Rescale = data[2][0] / data[0][0];
    double b21 = data[2][1] - data[0][1] * r2Rescale;
    double b22 = data[2][2] - data[0][2] * r2Rescale;
    double c2 = b.z - b.x * r2Rescale;

    if (-EPS < b11 && b11 < EPS) {
        return Vec3(NAN, NAN, NAN);
    }

    double zRescale = b21 / b11;

    struct Vec3 v;
    v.z = (c2 - c1 * zRescale) / (b22 - b12 * zRescale);
    v.y = (c1 - b12 * v.z) / b11;
    v.x = (b.x - data[0][1] * v.y - data[0][2] * v.z) / data[0][0];

    return v;
}
