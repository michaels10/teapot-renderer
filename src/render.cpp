#include "render.h"

const float inf = std::numeric_limits<float>::infinity();
const float PI = 3.1415926;
const int BLOCK_SIZE = 64;
const int PIXEL_BLOCK_SIZE = 2 * BLOCK_SIZE / sizeof(float);

Triangle const operator-(const Triangle &tri, const Vec3 &vec) {
  return Triangle(tri.v0 - vec, tri.v1 - vec, tri.v2 - vec, tri.normal);
}

Triangle const operator+(const Triangle &tri, const Vec3 &vec) {
  return Triangle(tri.v0 + vec, tri.v1 + vec, tri.v2 + vec, tri.normal);
}

void Camera::expose(Canvas &canvas) {
  if (exposure_mode == AUTO_LINEAR_EXPOSURE) {
    float minv = inf;
    float maxv = -inf;
    for (int i = 0; i < canvas.height; i++) {
      for (int j = 0; j < canvas.height; j++) {
        minv = min(minv, canvas[i][j]);
        maxv = min(maxv, canvas[i][j]);
      }
    }
    float scale = maxv - minv;
    for (int i = 0; i < canvas.height; i++) {
      for (int j = 0; j < canvas.height; j++) {
        canvas[i][j] = (canvas[i][j] - minv) / scale;
      }
    }
  } else {
    for (int i = 0; i < canvas.height; i++) {
      for (int j = 0; j < canvas.height; j++) {
        canvas[i][j] = canvas[i][j] / max_exposure_energy;
      }
    }
  }
}

RaycastResult raycast(const Vec3 &origin, const Vec3 &ray,
                      const Triangle &tri) {
  // Now the eq is to check if Apt + Bqt + Crt = D
  // Rewriting this: (a^x) t + a^o = D gives us t = D / (a^t x)
  float F = tri.normal ^ ray;
  if (fabs(F) < EPS) {
    return RaycastResult(false);
  }

  // Construct the point-norm eq Ax + By + Cz = D
  float t = ((tri.v0 - origin) ^ tri.normal) / F;
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

RaycastResult intersect(const Scene &scene, const Vec3 &origin,
                        const Vec3 &ray) {
  RaycastResult best_raycast(false);
  for (const Triangle &tri : scene.geometry) {
    RaycastResult res = raycast(origin, ray, tri);
    if (res.hit &&
        (res.distance < best_raycast.distance || !best_raycast.hit)) {
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

float fresnel(const Ray &incident, const RaycastResult &intersect) {
  float cosi = incident.ray ^ intersect.triangle.normal;
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
    float Rs =
        ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
    float Rp =
        ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
    return (Rs * Rs + Rp * Rp) / 2;
  }
}

Ray refract(const Ray &incident, const RaycastResult &intersect) {
  Ray refract_ray;
  refract_ray.refraction_index = intersect.triangle.refraction_index;
  refract_ray.origin = intersect.intersect;
  float c = intersect.triangle.normal ^ incident.ray;
  float r = incident.refraction_index / intersect.triangle.refraction_index;
  Vec3 refraction =
      (r * incident.ray) +
      (r * c - sqrt(1 - r * r * (1 - c * c))) * intersect.triangle.normal;
  refract_ray.ray = refraction;
  if ((refract_ray.ray ^ incident.ray) <= 0) {
    refract_ray.ray = -refract_ray.ray;
  }
  return refract_ray;
}

Ray reflect(const Ray &incident, const RaycastResult &intersect) {
  Ray reflect_ray;
  reflect_ray.refraction_index = incident.refraction_index;
  reflect_ray.origin = intersect.intersect;
  reflect_ray.ray =
      incident.ray - 2 * (incident.ray ^ intersect.triangle.normal) *
                         intersect.triangle.normal;
  return reflect_ray;
}

void render_ray(Canvas &canvas, const Scene &scene, const Ray &ray, int i,
                int j, float multiplier, int reflection_count,
                int max_reflections) {
  if (reflection_count >= max_reflections || multiplier < EPS) {
    return;
  }
  RaycastResult hit = intersect(scene, ray.origin, ray.ray);
  if (hit.hit) {
    canvas[i][j] += local_illuminate(hit, scene) * hit.triangle.scattering;
    if (hit.triangle.scattering + EPS < 1) {
      float fresnel_intensity = 1 - hit.triangle.scattering;
      float reflection_intensity = fresnel(ray, hit);
      Ray reflection_ray = reflect(ray, hit);
      render_ray(canvas, scene, reflection_ray, i, j,
                 multiplier * fresnel_intensity * reflection_intensity,
                 reflection_count + 1, max_reflections);
      if (reflection_intensity + EPS < 1.0) {
        float refraction_intensity = 1 - reflection_intensity;
        Ray refraction_ray = refract(ray, hit);
        render_ray(canvas, scene, refraction_ray, i, j,
                   multiplier * refraction_intensity * fresnel_intensity,
                   reflection_count + 1, max_reflections);
      }
    }
  }
}

Ray get_initial_ray(const Canvas &canvas, const Camera &camera, int ray_id) {
  int i = ray_id / canvas.width;
  int j = ray_id % canvas.width;
  Ray ray;
  ray.origin = camera.loc;
  float scaled_x = (j - canvas.width / 2.0) / 2.0 * camera.focal_plane_width;
  float scaled_y = (canvas.height / 2.0 - i) / 2.0 * camera.focal_plane_height;
  ray.ray = Vec3(scaled_x, scaled_y, camera.focal_plane_distance).normalize();
  ray.ray = ray.ray.rotate(camera.rotation);
  return ray;
}

void subrender(Canvas &canvas, const Scene &scene, const Camera &camera,
               queue<int> &block_queue, mutex &queue_lock) {
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
    int max_px =
        min(start_ray_id + PIXEL_BLOCK_SIZE, canvas.width * canvas.height);
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
  queue<int> blocks;
  mutex queue_lock;
  for (int i = 0; i < canvas.width * canvas.height; i += PIXEL_BLOCK_SIZE) {
    blocks.push(i);
  }
  vector<thread> threads;
  for (int i = 0; i < n_workers; i++) {
    threads.push_back(thread(subrender, ref(canvas), ref(scene), ref(camera),
                             ref(blocks), ref(queue_lock)));
  }
  for (thread &t : threads) {
    t.join();
  }
  camera.expose(canvas);
}

// Helper function for ensmallen, which is fed a range of coordinates,
// and returns the scaled sum of the values from in_arr in that range
// Requires range and the dimensions of the input array
float sum_scaled_range(float *in_arr, const float &x1, const float &y1,
                       const float &x2, const float &y2, const int dim_x,
                       const int dim_y) {
  // printf("---------(%d, %d) -> (%d, %d)---------\n", x1, y1, x2, y2);
  float ret = 0.0;
  for (int i = floor(x1); i < ceil(x2); i++) {
    for (int j = floor(y1); j < ceil(y2); j++) {
      float add = *(in_arr + (i * dim_y) + j);
      // printf("add: %f", add);
      if (i == floor(x1) && (float)(i) != x1) {
        add *= 1 - x1 + floor(x1);
      } else if (i == floor(x2) && (float)(i) != x2) {
        add *= x2 - floor(x2);
      }
      if (j == floor(y1) && (float)(j) != y1) {
        add *= 1 - y1 + floor(y1);
      } else if (j == floor(y2) && (float)(j) != y2) {
        add *= y2 - floor(y2);
      }
      ret += add;
      // printf("(%d, %d): %f\n", i, j, add);
    }
  }
  // printf("pre-factored add point = %f\n", ret);
  ret /= ((x2 - x1) * (y2 - y1));
  // printf("factor = %f\n", ((x2 - x1) * (y2 - y1)));
  // printf("final add point = %f\n", ret);
  return ret;
}

void Canvas::ensmallen(const int &out_x, const int &out_y) {
  float *out_arr = new float[out_x * out_y];
  memset(out_arr, 0.0f, width * height * sizeof(float));
  float x_scale = (float)(this->width) / (float)(out_x);
  float y_scale = (float)(this->height) / (float)(out_y);
  for (int i = 0; i < out_x; i++) {
    for (int j = 0; j < out_y; j++) {
      float x1 = i * x_scale;
      float y1 = j * y_scale;
      float x2 = x1 + x_scale;
      float y2 = y1 + y_scale;
      // printf("range: (%d, %d) -> (%d, %d)\n", x1, y1, x2, y2);
      *(out_arr + i * out_y + j) = sum_scaled_range(
          this->buffer, x1, y1, x2, y2, this->width, this->height);
    }
  }
  delete[] this->buffer;
  this->buffer = out_arr;
  this->height = out_y;
  this->width = out_x;
  // Visual degbugger, uncomment for terminal output of all forms of output
  // array kConstOutputSelector sel = IMAGE; term_out_image(*out_arr, out_x,
  // out_y, "final image", sel); sel = FLOAT; term_out_image(*out_arr, out_x,
  // out_y, "float image", sel); sel = NUM; term_out_image(*out_arr, out_x,
  // out_y, "number image", sel);
}

void Canvas::term_out_image(const char *title,
                            const kConstOutputSelector &selector) {
  printf("_________%s_________\n", title);
  for (int i = 0; i < this->width; i++) {
    for (int j = 0; j < this->height; j++) {
      switch (selector) {
      case FLOAT:
        printf("%f ", *(this->buffer + (i * this->height) + j));
        break;
      case NUM:
        printf("%d ", (int)(*(this->buffer + (i * this->height) + j) * 70.0));
        break;
      case IMAGE:
        printf("%c ",
               *(kConstBrightnessGradient +
                 (int)(*(this->buffer + (i * this->height) + j) * 70.0)));
      }
    }
    printf("\n");
  }
  printf("\n\n");
}