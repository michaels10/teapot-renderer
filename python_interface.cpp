#include "render.h"
#include "python_interface.h"

Scene scene;
Canvas canvas(300, 300);


extern "C" void add_triangle(PyTriangle tri) {
    Vec3 v0(tri.v0.x, tri.v0.y, tri.v0.z);
    Vec3 v1(tri.v1.x, tri.v1.y, tri.v1.z);
    Vec3 v2(tri.v2.x, tri.v2.y, tri.v2.z);
    //Vec3 normal(tri.normal.x, tri.normal.y, tri.normal.z);
    scene.geometry.push_back(Triangle(v0, v1, v2));
}

extern "C" void add_light(PyLight pylight) {
    Light light;
    light.loc =  Vec3(pylight.loc.x, pylight.loc.y, pylight.loc.z);
    light.intensity = pylight.intensity;
    scene.lights.push_back(light);
}

extern "C" PyCanvas render() {
    cout << "Rendering..." << endl;
    render(canvas, scene);
    PyCanvas pycanvas;
    pycanvas.width = canvas.width;
    pycanvas.height = canvas.height;
    pycanvas.canvas = canvas.buffer;
    return pycanvas;
}

int main(){
    return 0;
}