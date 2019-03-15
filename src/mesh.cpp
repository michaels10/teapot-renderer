#include "mesh.h"

RaycastResult Mesh::raycast(const Vec3 &origin, const Vec3 &ray, const Triangle &tri) const {
    Vec3 v0 = verts[tri.v0];
    Vec3 v1 = verts[tri.v1];
    Vec3 v2 = verts[tri.v2];
    Vec3 normal = normals[tri.normal];
    // 1: Determine ray-plane intersection.
    // Construct the point-norm eq Ax + By + Cz + D = 0
    float D = -(v0 ^ normal); // hehe float D
    // Solve Ax + By + Cz + D = 0, where (x,y,z) = tR+O
    // Can factor out the O:
    float Ds = D + (normal ^ origin);
    // Now the eq is to check if Apt + Bqt + Crt = -D
    // We can merge Ap, Bq, Cr = F to get Ft = -D
    float F = normal ^ ray;
    float t = -Ds / F;
    // Vec3 i = (t * ray) + origin;
    // If F is zero then the plane and ray are parallel
    // If t is negative, then there is no intersection
    if (fabs(F) < EPS || t < EPS) {
        return RaycastResult(false);
    }
    Vec3 intersect = origin + (ray * t);
    // Determine crux  angles.
    Vec3 A = v1 - v0;
    Vec3 B = v2 - v1;
    Vec3 C = v0 - v2;
    Vec3 AP = intersect - v0;
    Vec3 BP = intersect - v1;
    Vec3 CP = intersect - v2;
    // Now check if intersection point is in the interior of the v1 crux angle
    if (((A % AP) ^ normal) < 0) {
        return RaycastResult(false);
    }
    if (((B % BP) ^ normal) < 0) {
        return RaycastResult(false);
    }
    if (((C % CP) ^ normal) < 0) {
        return RaycastResult(false);
    }
    // RaycastResult(Vec3 intersect, Vec3 normal, float scattering, float refraction, float ior, float t) {
    return RaycastResult(intersect, normal, scattering, ior, t);
}
RaycastResult Mesh::intersect(const Vec3 &origin, const Vec3 &ray) const {
    RaycastResult best_raycast(false);
    for (const Triangle &tri : tris) {
        RaycastResult res = raycast(origin, ray, tri);
        if (res.hit) {
            if (!best_raycast.hit || res.distance < best_raycast.distance) {
                best_raycast = res;
            }
        }
    }

    return best_raycast;
}