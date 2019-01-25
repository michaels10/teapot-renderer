#include "render.h"

const float PI = 3.1415926;
const float EPS = 0.00001;
const float OBJECT_DIFFUSE = 0.95;
const float OBJECT_REFLECT = 0.05;

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
bool Vec3::operator==(const Vec3 &other) { return x == other.x && y == other.y && z == other.z; }
float Vec3::magnitude() const { return sqrt(x * x + y * y + z * z); }
Vec3 Vec3::normalize() const {
    float mag = magnitude();
    return Vec3(x / mag, y / mag, z / mag);
}
float Vec3::dot(const Vec3 &other) const { return other.x * x + other.y * y + other.z * z; }
float Vec3::sum() const { return x + y + z; }
Vec3 operator+(const Vec3 &v0, const Vec3 &v1) {
    Vec3 v;
    v.x = v0.x + v1.x;
    v.y = v0.y + v1.y;
    v.z = v0.z + v1.z;
    return v;
}
Vec3 const operator-(const Vec3 &v0, const Vec3 &v1) {
    Vec3 v;
    v.x = v0.x - v1.x;
    v.y = v0.y - v1.y;
    v.z = v0.z - v1.z;
    return v;
}
Vec3 const operator-(const Vec3 &v0) { return Vec3(-v0.x, -v0.y, -v0.z); }
Vec3 const operator*(const Vec3 &v0, const Vec3 &v1) {
    Vec3 v;
    v.x = v0.x * v1.x;
    v.y = v0.y * v1.y;
    v.z = v0.z * v1.z;
    return v;
}
Vec3 const operator*(const Vec3 &v0, float s) {
    Vec3 v;
    v.x = v0.x * s;
    v.y = v0.y * s;
    v.z = v0.z * s;
    return v;
}
Vec3 const operator*(float s, const Vec3 &v0) { return v0 * s; }
Vec3 const operator/(const Vec3 &v0, float s) {
    Vec3 v;
    v.x = v0.x / s;
    v.y = v0.y / s;
    v.z = v0.z / s;
    return v;
}
Vec3 const operator/(float s, const Vec3 &v0) { return v0 / s; }
/**
 * This does the cross-product of two vectors.
 **/
Vec3 const operator%(const Vec3 &v0, const Vec3 &v1) {
    Vec3 v;
    v.x = v0.y * v1.z - v0.z * v1.y;
    v.y = v0.z * v1.x - v0.x * v1.z;
    v.z = v0.x * v1.y - v0.y * v1.x;
    return v;
}

/**
 * This does the dot product of two vectors.
 **/
float const operator^(const Vec3 &v0, const Vec3 &v1) { return v0.dot(v1); }

Triangle const operator-(const Triangle &tri, const Vec3 &vec) { return Triangle(tri.v0 - vec, tri.v1 - vec, tri.v2 - vec, tri.normal); }
Triangle const operator+(const Triangle &tri, const Vec3 &vec) { return Triangle(tri.v0 + vec, tri.v1 + vec, tri.v2 + vec, tri.normal); }

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

RaycastResult raycast(const Vec3 &origin, const Vec3 &ray, const Triangle &tri) {
    // 1: Determine ray-plane intersection.
    // Construct the point-norm eq Ax + By + Cz + D = 0
    float D = -(tri.v0 ^ tri.normal); // hehe float D
    // Solve Ax + By + Cz + D = 0, where (x,y,z) = tR+O
    // Can factor out the O:
    float Ds = D + (tri.normal ^ origin);
    // Now the eq is to check if Apt + Bqt + Crt = -D
    // We can merge Ap, Bq, Cr = F to get Ft = -D
    float F = tri.normal ^ ray;
    float t = -Ds / F;
    Vec3 i = (t * ray) + origin;
    // If F is zero then the plane and ray are parallel
    // If t is negative, then there is no intersection
    if (fabs(F) < EPS || t < EPS) {
        return RaycastResult(false);
    }
    Vec3 intersect = origin + ray * t;
    // Determine barycentric basis.
    Vec3 A = tri.v1 - tri.v0;
    Vec3 B = tri.v2 - tri.v1;
    Vec3 C = tri.v0 - tri.v2;
    Vec3 AP = intersect - tri.v0;
    Vec3 BP = intersect - tri.v1;
    Vec3 CP = intersect - tri.v2;
    // Now check if intersection point is in the interior of the v1 crux angle
    if (((A % AP) ^ tri.normal) < 0) {
        return RaycastResult(false);
    }
    if (((B % BP) ^ tri.normal) < 0) {
        return RaycastResult(false);
    }
    if (((C % CP) ^ tri.normal) < 0) {
        return RaycastResult(false);
    }
    return RaycastResult(intersect, t, tri);
}

RaycastResult intersect(const Scene &scene, const Vec3 &origin, const Vec3 &ray) {
    RaycastResult best_raycast(false);
    for (const Triangle &tri : scene.geometry) {
        RaycastResult res = raycast(origin, ray, tri);
        if (res.hit && (res.distance < best_raycast.distance || !best_raycast.hit)) {
            best_raycast = res;
        }
    }
    return best_raycast;
}

bool any_intersect(const Scene &scene, const Vec3 &origin, const Vec3 &ray) {
    // fudge the origin a little bit to prevent same-hit intersection
    Vec3 new_origin = origin + (0.01 * ray);
    for (const Triangle &tri : scene.geometry) {
        RaycastResult res = raycast(new_origin, ray, tri);
        if (res.hit) {
            return true;
        }
    }
    return false;
}

float local_illuminate(const RaycastResult &hit, const Scene &scene) {
    // distance falloff only
    float total_illumination = 0;
    for (const Light &light : scene.lights) {
        Vec3 shadow_ray = (light.loc - hit.intersect);
        float dist = shadow_ray.magnitude();
        shadow_ray = shadow_ray.normalize();
        if (!any_intersect(scene, hit.intersect, shadow_ray)) {
            float intensity = light.intensity / (4 * PI * dist * dist);
            total_illumination += intensity;
        }
    }
    return total_illumination;
}

float fresnel(const RayTask &task, const RaycastResult &intersect) {
    float cosi = task.ray ^ intersect.triangle.normal;
    float etai = 1;
    float etat = intersect.triangle.refraction_index;
    if (cosi > 0) {
        swap(etai, etat);
    }
    float sint = etai / etat * sqrt(max(0.f, 1 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1) {
        return 1;
    } else {
        float cost = sqrt(max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        return (Rs * Rs + Rp * Rp) / 2;
    }
}

RayTask refract(const RayTask &task, const RaycastResult &intersect, float intensity) {
    RayTask refract_task;
    refract_task.bounce_count = task.bounce_count + 1;
    refract_task.i = task.i;
    refract_task.j = task.j;
    refract_task.multiplier = task.multiplier * intensity;
    refract_task.origin = intersect.intersect;

    float c = -intersect.triangle.normal.sum();
    float r = task.refraction_index / intersect.triangle.refraction_index;
    Vec3 v1 = Vec3(1, 1, 1);
    Vec3 refraction = (r * v1) + (r * c - sqrt(1 - r * r * (1 - c * c))) * intersect.triangle.normal;
    refract_task.ray = refraction;
    return refract_task;
}

RayTask reflect(const RayTask &task, const RaycastResult &intersect, float intensity) {
    RayTask reflect_task;
    reflect_task.bounce_count = task.bounce_count + 1;
    reflect_task.i = task.i;
    reflect_task.j = task.j;
    reflect_task.multiplier = task.multiplier;
    reflect_task.origin = intersect.intersect;
    reflect_task.ray = task.ray - 2 * (task.ray ^ intersect.triangle.normal) * intersect.triangle.normal;
    return reflect_task;
}

vector<RayTask> subrender(Canvas &canvas, const Scene &scene, const RayTask &task, int max_reflections) {
    vector<RayTask> new_rays;
    if (task.bounce_count < max_reflections) {
        RaycastResult hit = intersect(scene, task.origin, task.ray);
        if (hit.hit) {
            canvas[task.i][task.j] += local_illuminate(hit, scene) * hit.triangle.scattering;
            if (hit.triangle.scattering < 1) {
                float fresnel_intensity = 1-hit.triangle.scattering;
                float reflection_intensity = fresnel(task, hit);
                new_rays.push_back(reflect(task, hit, reflection_intensity*fresnel_intensity));
                new_rays.push_back(refract(task, hit, (1 - reflection_intensity)*fresnel_intensity));
            }
        }
    }
    return new_rays;
}

vector<RayTask> get_initial_rays(Canvas &canvas) {
    Vec3 camera_loc(0, -4, -10);
    float camera_width = 15;
    float camera_height = 15;
    float focal_plane = 1000;
    vector<RayTask> tasks;
    for (int i = 0; i < canvas.height; i++) {
        for (int j = 0; j < canvas.width; j++) {
            RayTask task;
            task.bounce_count = 0;
            task.i = i;
            task.j = j;
            task.multiplier = 1;
            task.origin = camera_loc;
            float scaled_j = (j - canvas.width / 2) * camera_width;
            float scaled_i = -(i - canvas.height / 2) * camera_height;
            task.ray = Vec3(camera_loc.x + scaled_j, camera_loc.y + scaled_i, camera_loc.z + focal_plane);
            task.ray = task.ray - camera_loc;
            task.ray = task.ray.normalize();
            tasks.push_back(task);
        }
    }
    return tasks;
}

void render(Canvas &canvas, const Scene &scene) {
    queue<RayTask> tasks;
    for (RayTask task : get_initial_rays(canvas)) {
        tasks.push(task);
    }

    while (!tasks.empty()) {
        RayTask task = tasks.front();
        tasks.pop();
        for (RayTask task : subrender(canvas, scene, task, 2)) {
            tasks.push(task);
        }
    }
}