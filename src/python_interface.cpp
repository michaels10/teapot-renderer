#include "python_interface.h"

extern "C" void add_mesh(PyScene *scene, float *vert, size_t vert_len, 
		int *tris, size_t tris_len, float scttrng, float ior) {
    Mesh mesh = Mesh(vert, vert_len, tris, tris_len, scttrng, ior);
    ((Scene*)(scene->scene))->geometry.push_back(mesh);
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
    scene->scene = new Scene(); 
}

extern "C" void __init_canvas(PyCanvas *canvas, int width, int height) {
    canvas->cpp_canvas = new Canvas(height, width);
    canvas->canvas = ((Canvas*)canvas->cpp_canvas)->buffer;
    canvas->width = width;
    canvas->height = height;
}

extern "C" void render(PyScene* scene, PyCanvas* canvas) {
    render(*(Canvas*)(canvas->cpp_canvas), *(Scene*)(scene->scene), Camera());
}
