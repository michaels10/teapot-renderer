#include "mesh.h"

RaycastResult Mesh::raycast(const Vec3 &origin, const Vec3 &ray, const Triangle &tri) {
    // Now the eq is to check if Apt + Bqt + Crt = D
    // Rewriting this: (a^x) t + a^o = D gives us t = D / (a^t x)
    float F = normals[tri.normal]^ray;
    if (fabs(F) < EPS) return RaycastResult(false);

    // Construct the point-norm eq Ax + By + Cz = D
    float t = ((verts[tri.v0] - origin) ^ tri.normal) / F;
    // If t is negative, then there is no intersection
    if (t < EPS) return RaycastResult(false);

    Vec3 intersect = (ray * t) - verts[tri.v0] + origin;
    Vec3 v0 = verts[tri.v1] - verts[tri.v0];
    Vec3 v1 = verts[tri.v2] - verts[tri.v0];
    float gamma = (v0 ^ v1) / (v0 ^ v0);
    Vec3 v2 = v1 - gamma * v0;

    // Coordinates in triangle-basis
    float b = (v2 ^ intersect) / (v2 ^ v2);
    float a = (intersect ^ v0) / (v0 ^ v0) - b * gamma;

    if (a < 0 || b < 0) return RaycastResult(false);
    if (a + b > 1) return RaycastResult(false);

    return RaycastResult(intersect + verts[tri.v0], normals[tri.normal], scattering,
        refraction, ior, t);
}

RaycastResult Mesh::intersect(const Vec3 &origin, const Vec3 &ray) {
    RaycastResult best_raycast(false);
    for (const Triangle &tri : tris) {
        RaycastResult res = raycast(origin, ray, tri);
        if (res.hit) {
            if (!best_raycast.hit || res.distance < best_raycast.distance ) {
                best_raycast = res;
            }
        }
    }
    return best_raycast;
}