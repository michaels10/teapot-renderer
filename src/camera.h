#ifndef CAMERA_H
#define CAMERA_H

#include "linalg.h"
#include "scene.h"

const int AUTO_LINEAR_EXPOSURE = 0;
const int MANUAL_LINEAR_EXPOSURE = 1;

struct Camera {
  public:
    Vec3 loc = Vec3(0, 4, -10);
    Vec3 rotation; // = Vec3(-0.523599, 0, 0); // PYR, default is looking down Z+

    // Camera settings
    float focal_plane_distance = 1;
    float focal_plane_width = 4;
    float focal_plane_height = 4;
    int exposure_mode = AUTO_LINEAR_EXPOSURE;
    float max_exposure_energy = 55.0f;

    // Render settings
    float multiplier = 1;
    int reflection_count = 1, max_reflections = 8;

    Camera(float focal_distance, float width, float height, float max_exposure, int res_x, int res_y) {
        this->focal_plane_distance = focal_distance;
        this->focal_plane_width = width;
        this->focal_plane_height = height;
        this->max_exposure_energy = max_exposure;
        canvas = Canvas(res_x, res_y);

        exposure_mode = MANUAL_LINEAR_EXPOSURE;
    }

    Camera(float focal_distance, float width, float height, int res_x, int res_y) {
        this->focal_plane_distance = focal_distance;
        this->focal_plane_width = width;
        this->focal_plane_height = height;

        exposure_mode = AUTO_LINEAR_EXPOSURE;
    }

    void expose(Canvas &canvas);
    Ray get_initial_ray(int i, int j) const;
    void render_ray(const Scene &scene, const Ray &ray, int i, int j, 
        double multiplier, int reflection_count);
};

#endif