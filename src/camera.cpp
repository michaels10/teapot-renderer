#include "scene.h"

const float inf = std::numeric_limits<float>::infinity();

void Camera::expose(Canvas &canvas) {
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

Ray Camera::get_initial_ray(int i, int j, const Canvas& canvas) const {
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