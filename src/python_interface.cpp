#include "python_interface.h"

extern "C" void add_mesh(PyScene *scene, float *vert, size_t vert_len, int *tris, size_t tris_len, float scattering, float ior) {
    Mesh mesh(vert, vert_len, tris, tris_len, scattering, ior);
    ((Scene*)(scene->scene))->geometry.push_back(mesh);
    printf("Added %zu meshes to scene\n", ((Scene*)(scene->scene))->geometry.size());
}

extern "C" void add_light(PyScene * scene, PyLight *pylight) {
    Light light;
    light.loc = Vec3(pylight->loc.x, pylight->loc.y, pylight->loc.z);
    light.intensity = pylight->intensity;
    ((Scene*)(scene->scene))->lights.push_back(light);
}

extern "C" void __init_PyVec3(PyVec3 * vec, double x, double y, double z) {
    vec->x = x;
    vec->y = y;
    vec->z = z;
}

extern "C" void __init_scene(PyScene *scene) { 
    Scene* new_scene = new Scene();
    new_scene->max_reflections = 8;
    scene->scene = new_scene;
}

extern "C" void __init_canvas(PyCanvas *canvas, int width, int height) { 
    canvas->width = width;
    canvas->height = height;
    Canvas* cpp_canvas = new Canvas(width, height);
    canvas->cpp_canvas = cpp_canvas;
    canvas->canvas = cpp_canvas->buffer;
}

extern "C" void render(PyScene* scene, PyCanvas* canvas) {
    Scene* cpp_scene = (Scene*) scene->scene;
    Canvas* cpp_canvas = (Canvas*)(canvas->cpp_canvas);
    cpp_scene->render(*cpp_canvas);
}
