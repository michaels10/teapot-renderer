#include <ncurses.h>
#include <stdio.h>
#include <math.h>
#define EPS .0000000000001

////////////////////////////////// VECTORS /////////////////////////////////////

struct Vec3 {
    double x, y, z;
};
    
struct Vec4 {
    double i, j, k, r;
};


struct Vec3 m_vec(double x, double y, double z) {
    struct Vec3 vec;
    vec.x = x;
    vec.y = y;
    vec.z = z;
    return vec;
}

struct Vec3 rescale(struct Vec3 vec, double scale) {
    return m_vec(vec.x * scale, vec.y * scale, vec.z * scale);
}

struct Vec3 add(struct Vec3 v0, struct Vec3 v1) {
    return m_vec(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}

struct Vec3 sub(struct Vec3 v0, struct Vec3 v1) {
    return m_vec(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}

double dot(struct Vec3 v0, struct Vec3 v1) {
    return v0.x*v1. x + v0.y*v1.y + v0.z*v1.z;
}

double mag(struct Vec3 vec) { 
    return sqrt(dot(vec, vec));
}

struct Vec3 cross(struct Vec3 v0, struct Vec3 v1) {
    struct Vec3 vec;
    vec.x = v0.y * v1.z - v0.z * v1.y;
    vec.y = v0.z * v1.x - v0.x * v1.z;
    vec.z = v0.x * v1.y - v0.y * v1.x;
    return vec;
}

void print_Vec3(struct Vec3 vector) {
    printf("\n< %5.2f, %5.2f, %5.2f >", vector.x, vector.y, vector.z);
}

//////////////////////////// MATRICES /////////////////////////////////////////
struct Mat3x3 {
    struct Vec3 v0, v1, v2;
};

struct Mat3x3 m_mat(struct Vec3 v0, struct Vec3 v1, struct Vec3 v2) {
    struct Mat3x3 mat;
    mat.v0 = v0;
    mat.v1 = v1;
    mat.v2 = v2;
    return mat;
}

// Matrix determinant
double Det3(struct Mat3x3 A) {
    double cofactor1 = A.v0.x * (A.v1.y * A.v2.z - A.v1.z * A.v2.y);
    double cofactor2 = A.v1.x * (A.v0.y * A.v2.z - A.v0.z * A.v2.y);
    double cofactor3 = A.v2.x * (A.v0.y * A.v1.z - A.v0.z * A.v1.y);
    return cofactor1 - cofactor2 + cofactor3;
}

// Solves a given 3x3 matrix for a given output.
// Returns:
//      x from Ax = b
//      Fills x with NaN if it fails for whatever reason.
//
// Note: 
//      Does not deal with row transposition. Can be mostly avoided by having 
//      symmetric camera pixels.
struct Vec3 solve3Mat(struct Mat3x3 A, struct Vec3 b ) {
    double det = Det3(A);
    if((-EPS < det && det < EPS) || A.v0.x == 0) {
        return m_vec(NAN, NAN, NAN);
    }

    double r1Rescale = A.v0.y / A.v0.x;
    double b11 = A.v1.y - A.v1.x * r1Rescale;
    double b12 = A.v2.y - A.v2.x * r1Rescale;
    double c1 = b.y - b.x * r1Rescale;

    double r2Rescale = A.v0.z / A.v0.x;
    double b21 = A.v1.z - A.v1.x * r2Rescale;
    double b22 = A.v2.z - A.v2.x * r2Rescale;
    double c2 = b.z - b.x * r2Rescale;

    if (-EPS < b11 && b11 < EPS) {
        return m_vec(NAN, NAN, NAN);
    }

    double zRescale =  b21 / b11;

    struct Vec3 v;
    v.z = (c2 - c1 * zRescale) / (b22 - b12 * zRescale);
    v.y = (c1 - b12 * v.z) / b11;
    v.x = (b.x - A.v1.x * v.y - A.v2.x * v.z ) / A.v0.x;

    return v;
}

void print_Mat3x3(struct Mat3x3 matrix) {
    printf("\n| %5.2f, %5.2f, %5.2f |", matrix.v0.x, matrix.v1.x, matrix.v2.x);
    printf("\n| %5.2f, %5.2f, %5.2f |", matrix.v0.y, matrix.v1.y, matrix.v2.y);
    printf("\n| %5.2f, %5.2f, %5.2f |", matrix.v0.z, matrix.v1.z, matrix.v2.z);
}

////////////////////////// MESH LIB ///////////////////////////////////////

struct Triangle {
    struct Vec3 v0, v1, v2, normal;
};

// Casts a ray from origin (at camera) that figures out if you're gonna hit the triangle.
bool raycast(struct Vec3 p_pos, struct Triangle shifted_tri) {
    struct Vec3 solved = solve3Mat(m_mat(p_pos, shifted_tri.v1, shifted_tri.v2), shifted_tri.v0);
    if(solved.x == NAN || solved.y == NAN || solved.z == NAN) {
        return false;
    }
    bool non_0 = solved.x > EPS && solved.y > EPS && solved.z > EPS;
    bool in_tri = solved.y <= 1 && solved.z <= 1;
    return non_0 && in_tri;
}

char do_lighting(struct Triangle tri, struct Vec3 light_dir) {
    char DENSITIES[4] = {(char)176, (char)177, (char)178, (char)219};

    struct Vec3 rescaled_normal = rescale(tri.normal, mag(tri.normal));
    double parallel_measure = dot(rescaled_normal, light_dir);
    parallel_measure = (parallel_measure + 1) / 2.;

    return DENSITIES[1 + (int) (parallel_measure * (4 - 1)) ];
}

// Assuming that proper coordingate transformations have been performed, cast rays at 
// the triangle.
char render_object(struct Vec3 ray, struct Triangle * shift_tris, int tri_len) {
    for(int i = 0; i < tri_len; i++) {
        bool light = raycast(ray, shift_tris[i]);
    }
    return 'a';
}

void render(char * name) {
    
}


int main() {
    render("UtahTeapot.stl");
    return 0;
}
