#include "render.h"

const float PI = 3.1415926;
const float OBJECT_DIFFUSE = 0.95;
const float OBJECT_REFLECT = 0.05;
const int CONCURRENT_BATCH_SIZE = 128*1;

Triangle const operator-(const Triangle &tri, const Vec3 &vec) {
    return Triangle(tri.v0 - vec, tri.v1 - vec, tri.v2 - vec, tri.normal);
}

Triangle const operator+(const Triangle &tri, const Vec3 &vec) {
    return Triangle(tri.v0 + vec, tri.v1 + vec, tri.v2 + vec, tri.normal);
}


RaycastResult raycast(const Vec3 &origin, const Vec3 &ray, const Triangle &tri) {
    // Now the eq is to check if Apt + Bqt + Crt = D
    // Rewriting this: (a^x) t + a^o = D gives us t = D / (a^t x)
    float F = tri.normal ^ ray;
    if (fabs(F) < EPS) {
        return RaycastResult(false);
    }

    // Construct the point-norm eq Ax + By + Cz = D
    float t = ((tri.v0 - origin) ^ tri.normal)/ F;
    // If t is negative, then there is no intersection
    if (t < EPS) {
        return RaycastResult(false);
    }

    Vec3 intersect = (ray * t) - tri.v0 + origin;
    Vec3 v0 = tri.v1 - tri.v0;
    Vec3 v1 = tri.v2 - tri.v0;
    float gamma = (v0 ^ v1) / (v0 ^ v0);
    Vec3 v2 = v1 - gamma * v0;
    
    // Coordinates in triangle-basis
    float b = (v2 ^ intersect) / (v2 ^ v2);
    float a = (intersect ^ v0) / (v0 ^ v0) - b * gamma;

    if (a < 0 || b < 0) {
        return RaycastResult(false);
    }
    if (a + b > 1) {
        return RaycastResult(false);
    }

    return RaycastResult(intersect + tri.v0, t, tri);
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
    Vec3 refraction =
        (r * v1) + (r * c - sqrt(1 - r * r * (1 - c * c))) * intersect.triangle.normal;
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
    reflect_task.ray =
        task.ray - 2 * (task.ray ^ intersect.triangle.normal) * intersect.triangle.normal;
    return reflect_task;
}

void subrender(Canvas &canvas, const Scene &scene, int max_reflections, RayQueue &ray_queue) {
    while (!ray_queue.empty()) {
        vector<RayTask> tasks = ray_queue.pop_batch(CONCURRENT_BATCH_SIZE);
        vector<RayTask> new_rays;
        for (const RayTask &task : tasks) {
            if (task.bounce_count < max_reflections) {
                RaycastResult hit = intersect(scene, task.origin, task.ray);
                if (hit.hit) {
                    canvas[task.i][task.j] +=
                        local_illuminate(hit, scene) * hit.triangle.scattering;
                    if (hit.triangle.scattering < 1) {
                        float fresnel_intensity = 1 - hit.triangle.scattering;
                        float reflection_intensity = fresnel(task, hit);
                        new_rays.push_back(
                            reflect(task, hit, reflection_intensity * fresnel_intensity));
                        new_rays.push_back(
                            refract(task, hit, (1 - reflection_intensity) * fresnel_intensity));
                    }
                }
            }
        }
        ray_queue.push_batch(new_rays);
    }
}

vector<RayTask> get_initial_rays(Canvas &canvas) {
    Vec3 camera_loc(3, 4, -4);
    float camera_width = 7;
    float camera_height = 7;
    float focal_plane = 3;
    vector<RayTask> tasks;
    for (int i = 0; i < canvas.height; i++) {
        for (int j = 0; j < canvas.width; j++) {
            RayTask task;
            task.bounce_count = 0;
            task.i = i;
            task.j = j;
            task.multiplier = 1;
            task.origin = camera_loc;
            float scaled_j = (j - canvas.width / 2.0) * camera_width;
            float scaled_i = -(i - canvas.height / 2.0) * camera_height;
            task.ray =
                Vec3(camera_loc.x + scaled_j, camera_loc.y + scaled_i, camera_loc.z + focal_plane);
            task.ray = task.ray - camera_loc;
            task.ray = task.ray.normalize();
            tasks.push_back(task);
        }
    }
    return tasks;
}

void render(Canvas &canvas, const Scene &scene) {
    RayQueue tasks(4096);
    int n_workers = max((int)thread::hardware_concurrency() - 1, 1);
    vector<thread> threads;
    tasks.push_batch(get_initial_rays(canvas));
    for (int i = 0; i < n_workers; i++) {
        threads.push_back(thread(subrender, ref(canvas), ref(scene), 3, ref(tasks)));
    }
    for (thread &t : threads) {
        t.join();
    }
}
