#include "python_interface.h"


extern "C" void add_triangle(PyTriangle *tri, PyScene *scene) {
    Vec3 v0(tri->v0.x, tri->v0.y, tri->v0.z);
    Vec3 v1(tri->v1.x, tri->v1.y, tri->v1.z);
    Vec3 v2(tri->v2.x, tri->v2.y, tri->v2.z);
    // Vec3 normal(tri.normal.x, tri.normal.y, tri.normal.z);
    scene->scene->geometry.push_back(Triangle(v0, v1, v2));
}

extern "C" void add_light(PyLight *pylight, PyScene *scene) {
    Light light;
    light.loc = Vec3(pylight->loc.x, pylight->loc.y, pylight->loc.z);
    light.intensity = pylight->intensity;
    scene->scene->lights.push_back(light);
}

extern "C" void __init_scene(PyScene *scene) { scene->scene = new Scene(); }

extern "C" void __init_canvas(PyCanvas *canvas, int width, int height) {
    canvas->cpp_canvas = new Canvas(height, width);
    canvas->canvas = canvas->cpp_canvas->buffer;
    canvas->width = width;
    canvas->height = height;
}

extern "C" void render(PyScene* scene, PyCanvas* canvas) {
    render(*canvas->cpp_canvas, *scene->scene);
}