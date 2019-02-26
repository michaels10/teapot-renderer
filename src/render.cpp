#include "render.h"
#include "octree.h"

const float inf = std::numeric_limits<float>::infinity();
const float PI = 3.1415926;
const int BLOCK_SIZE = 64;
const int PIXEL_BLOCK_SIZE = 2 * BLOCK_SIZE / sizeof(float);

BoundingBox Triangle::get_bounds() const{
    double min_x = min(v0.x, min(v1.x, v2.x));
    double min_y = min(v0.y, min(v1.y, v2.y));
    double min_z = min(v0.z, min(v1.z, v2.z));

    double max_x = max(v0.x, max(v1.x, v2.x));
    double max_y = max(v0.y, max(v1.y, v2.y));
    double max_z = max(v0.z, max(v1.z, v2.z));

    BoundingBox box;
    box.min_xyz = Vec3(min_x, min_y, min_z);
    box.max_xyz = Vec3(max_x, max_y, max_z);

    return box;
}

void Camera::expose(Canvas &canvas) const {
    float max_exposure = max_exposure_energy;
    switch (exposure_mode) {
    case AUTO_LINEAR_EXPOSURE: {
        max_exposure = -inf;
        for (int i = 0; i < canvas.height; i++) {
            for (int j = 0; j < canvas.height; j++) {
                max_exposure = max(max_exposure, canvas[i][j]);
            }
        }
        break;
    }
    case MANUAL_LINEAR_EXPOSURE:
        break;
    }
    for (int i = 0; i < canvas.height; i++) {
        for (int j = 0; j < canvas.height; j++) {
            canvas[i][j] = canvas[i][j] / max_exposure;
            if (canvas[i][j] > 1) canvas[i][j] = 1;
        }
    }
}

RaycastResult raycast(const Vec3 &origin, const Vec3 &ray, const Triangle &tri) {
    // Now the eq is to check if Apt + Bqt + Crt = D
    // Rewriting this: (a^x) t + a^o = D gives us t = D / (a^t x)
    float F = tri.normal ^ ray;
    if (fabs(F) < EPS) return RaycastResult(false);

    // Construct the point-norm eq Ax + By + Cz = D
    float t = ((tri.v0 - origin) ^ tri.normal) / F;
    // If t is negative, then there is no intersection
    if (t < EPS) return RaycastResult(false);

    Vec3 intersect = (ray * t) - tri.v0 + origin;
    Vec3 v0 = tri.v1 - tri.v0;
    Vec3 v1 = tri.v2 - tri.v0;
    float gamma = (v0 ^ v1) / (v0 ^ v0);
    Vec3 v2 = v1 - gamma * v0;

    // Coordinates in triangle-basis
    float b = (v2 ^ intersect) / (v2 ^ v2);
    float a = (intersect ^ v0) / (v0 ^ v0) - b * gamma;

    if (a < 0 || b < 0) return RaycastResult(false);
    if (a + b > 1) return RaycastResult(false);

    return RaycastResult(intersect + tri.v0, t, tri);
}

RaycastResult intersect(const Scene &scene, const Vec3 &origin, const Vec3 &ray) {
    RaycastResult best_raycast(false);
    for (const Mesh &mesh : scene.geometry) {
        for (const RefTriangle &tri : mesh.tris) {
            RaycastResult res = raycast(origin, ray, mesh.get_triangle(tri));
            if (res.hit && (res.distance < best_raycast.distance || !best_raycast.hit)) {
                best_raycast = res;
            }
	}
    }
    return best_raycast;
}

bool any_intersect(const Scene &scene, const Vec3 &origin, const Vec3 &ray) {
    // fudge the origin a little bit to prevent same-hit intersection
    Vec3 new_origin = origin + (0.01 * ray);
    for (const Mesh &mesh: scene.geometry) {
        for (const RefTriangle &tri : mesh.tris) {
            RaycastResult res = raycast(new_origin, ray, mesh.get_triangle(tri));
            if (res.hit) return true;
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

float fresnel(const Ray &incident, const RaycastResult &intersect) {
    float cosi = incident.ray ^ intersect.triangle.normal;
    float etai = 1;
    float etat = intersect.triangle.ior;
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

Ray refract(const Ray &incident, const RaycastResult &intersect) {
    Ray refract_ray;
    refract_ray.ior = intersect.triangle.ior;
    refract_ray.origin = intersect.intersect;
    float c = intersect.triangle.normal ^ incident.ray;
    float r = incident.ior / intersect.triangle.ior;
    Vec3 refraction = (r * incident.ray) + (r * c - sqrt(1 - r * r * (1 - c * c))) * intersect.triangle.normal;
    refract_ray.ray = refraction;
    if ((refract_ray.ray ^ incident.ray) <= 0) {
        refract_ray.ray = -refract_ray.ray;
    }
    return refract_ray;
}

Ray reflect(const Ray &incident, const RaycastResult &intersect) {
    Ray reflect_ray;
    reflect_ray.ior = incident.ior;
    reflect_ray.origin = intersect.intersect;
    reflect_ray.ray = incident.ray - 2 * (incident.ray ^ intersect.triangle.normal) * intersect.triangle.normal;
    return reflect_ray;
}

void render_ray(Canvas &canvas, const Scene &scene, const Ray &ray, int i, int j, float multiplier,
                int reflection_count, int max_reflections) {
    // Kick out if ray impact is negligible
    if (reflection_count >= max_reflections || multiplier < EPS) return;

    RaycastResult hit = intersect(scene, ray.origin, ray.ray);
    // Kick out if we aren't scattering
    if (!hit.hit) return;

    canvas[i][j] += local_illuminate(hit, scene) * hit.triangle.scattering;
    // Kick out if we already hit max brightness
    if (hit.triangle.scattering + EPS >= 1) return;

    float fresnel_intensity = 1 - hit.triangle.scattering;
    float reflection_intensity = fresnel(ray, hit);
    Ray reflection_ray = reflect(ray, hit);
    render_ray(canvas, scene, reflection_ray, i, j, multiplier * fresnel_intensity * reflection_intensity,
     	       reflection_count + 1, max_reflections);
    if (reflection_intensity + EPS < 1.0) {
        float refraction_intensity = 1 - reflection_intensity;
        Ray refraction_ray = refract(ray, hit);
	render_ray(canvas, scene, refraction_ray, i, j, multiplier * refraction_intensity * fresnel_intensity,
		   reflection_count + 1, max_reflections);
    }
}

Ray get_initial_ray(const Canvas &canvas, const Camera &camera, int ray_id) {
    int i = ray_id / canvas.width;
    int j = ray_id % canvas.width;
    Ray ray;
    ray.origin = camera.loc;
    int fold_i = canvas.height / 2.0;
    int fold_j = canvas.width / 2.0;
    float scaled_x = (j - fold_j) * camera.focal_plane_width / canvas.width;
    float scaled_y = (fold_i - i) * camera.focal_plane_height / canvas.height;
    ray.ray = Vec3(scaled_x, scaled_y, camera.focal_plane_distance).normalize();
    ray.ray = ray.ray.rotate(camera.rotation);
    return ray;
}

void subrender(Canvas &canvas, const Scene &scene, const Camera &camera, queue<int> &block_queue, mutex &queue_lock) {
    while (true) {
        queue_lock.lock();
        // printf("%lu render blocks remaining...\n", block_queue.size());
        if (block_queue.empty()) {
            queue_lock.unlock();
            break;
        }
        int start_ray_id = block_queue.front();
        block_queue.pop();
        queue_lock.unlock();
        int max_px = min(start_ray_id + PIXEL_BLOCK_SIZE, canvas.width * canvas.height);
        for (int ray_id = start_ray_id; ray_id < max_px; ray_id++) {
            int i = ray_id / canvas.width;
            int j = ray_id % canvas.width;
            Ray ray = get_initial_ray(canvas, camera, ray_id);
            render_ray(canvas, scene, ray, i, j, 1, 0, camera.max_reflections);
        }
    }
}

void render(Canvas &canvas, const Scene &scene, const Camera &camera) {
    int n_workers = max((int)thread::hardware_concurrency() - 1, 1);
    //Octree octo(scene);
    queue<int> blocks;
    mutex queue_lock;
    for (int i = 0; i < canvas.width * canvas.height; i += PIXEL_BLOCK_SIZE) {
        blocks.push(i);
    }
    vector<thread> threads;
    for (int i = 0; i < n_workers; i++) {
        threads.push_back(thread(subrender, ref(canvas), ref(scene), ref(camera), ref(blocks), ref(queue_lock)));
    }
    for (thread &t : threads) {
        t.join();
    }
    camera.expose(canvas);
}
