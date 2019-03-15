#include "render.h"

const float inf = std::numeric_limits<float>::infinity();
const float PI = 3.1415926;

void Camera::expose() {
    float max_exposure = max_exposure_energy;
    switch (exposure_mode) {
    case AUTO_LINEAR_EXPOSURE: {
        max_exposure = -inf;
        for (int i = 0; i < canvas.width; i++) {
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

RaycastResult all_intersect(const Scene &scene, const Vec3 &origin, const Vec3 &ray) {
    // fudge the origin a little bit to prevent same-hit intersection
    Vec3 new_origin = origin + (0.01 * ray);
    RaycastResult best_raycast(false);
    for (auto mesh: scene.geometry) {
        RaycastResult res = mesh.intersect(new_origin, ray);
        if (res.hit) {
            if (!best_raycast.hit || res.distance < best_raycast.distance ) {
                best_raycast = res;
            }
        }
    }
    return best_raycast;

}

bool any_intersect(const Scene &scene, const Vec3 &origin, const Vec3 &ray) {
    // fudge the origin a little bit to prevent same-hit intersection
    Vec3 new_origin = origin + (0.01 * ray);
    for (auto mesh: scene.geometry) {
        for (auto triangle: mesh.tris) {
            RaycastResult res = mesh.raycast(new_origin, ray, triangle);
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
    float cosi = incident.ray ^ intersect.normal;
    float etai = 1;
    float etat = intersect.ior;
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
    refract_ray.ior = intersect.ior;
    refract_ray.origin = intersect.intersect;
    float c = intersect.normal ^ incident.ray;
    float r = incident.ior / intersect.ior;
    Vec3 refraction = (r * incident.ray) + (r * c - sqrt(1 - r * r * (1 - c * c))) * intersect.normal;
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
    reflect_ray.ray = incident.ray - 2 * (incident.ray ^ intersect.normal) * intersect.normal;
    return reflect_ray;
}

void Camera::render_ray(const Scene &scene, const Ray &ray, int i, int j,
        double multiplier, int reflection_count) {
    // Kick out if too many reflections is negligible
    if (reflection_count >= max_reflections || multiplier < EPS) return;

    // make this mesh specific.
    RaycastResult hit = all_intersect(scene, ray.origin, ray.ray);
    // Kick out if we aren't scattering
    if (!hit.hit) return;

    canvas[i][j] += local_illuminate(hit, scene) * hit.scattering;
    // Kick out if we already hit max brightness
    if (hit.scattering + EPS >= 1) return;

    float fresnel_intensity = 1 - hit.scattering;
    float reflection_intensity = fresnel(ray, hit);
    Ray reflection_ray = reflect(ray, hit);
    render_ray(scene, reflection_ray, i, j, multiplier * fresnel_intensity * reflection_intensity,
     	       reflection_count + 1);
    if (reflection_intensity + EPS < 1.0) {
        float refraction_intensity = 1 - reflection_intensity;
        Ray refraction_ray = refract(ray, hit);
        render_ray(scene, refraction_ray, i, j, multiplier * refraction_intensity * fresnel_intensity,
		   reflection_count + 1);
    }
}

Ray Camera::get_initial_ray(int i, int j) const {
    Ray ray;
    ray.origin = loc;
    int fold_i = canvas.height / 2.0;
    int fold_j = canvas.width / 2.0;
    float scaled_x = (j - fold_j) * focal_plane_width / canvas.width;
    float scaled_y = (fold_i - i) * focal_plane_height / canvas.height;
    ray.ray = Vec3(scaled_x, scaled_y, focal_plane_distance).normalize();
    ray.ray = ray.ray.rotate(rotation);
    return ray;
}

void render(const Scene &scene, Camera &camera) {
    //Octree octo(scene);
    for (int i = 0; i < camera.canvas.width; i++) {
        for (int j = 0; j < camera.canvas.height; j++) {
            Ray ray = camera.get_initial_ray(i, j);
            camera.render_ray(scene, ray, i, j, 1, 2);
        }
    }
    camera.expose();
}
