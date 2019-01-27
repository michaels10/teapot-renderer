from cffi import FFI
import numpy as np
ffi = FFI()
ffi.cdef("""
typedef struct PyVec3 {
  float x, y, z;
} PyVec3;

typedef struct PyTriangle {
  PyVec3 v0;
  PyVec3 v1;
  PyVec3 v2;
  PyVec3 normal;
  float scattering;
  float refraction_index;
} PyTriangle;

typedef struct PyCanvas {
  int width;
  int height;
  float *canvas;
  void* cpp_canvas;
} PyCanvas;

typedef struct PyScene {
  void* scene;
} PyScene;

typedef struct PyLight {
  PyVec3 loc;
  float intensity;
} PyLight;

void add_triangle(PyTriangle *tri, PyScene *scene);
void add_light(PyLight *pylight, PyScene *scene);
void __init_scene(PyScene *scene);
void __init_canvas(PyCanvas *canvas, int width, int height);
void render(PyScene* scene, PyCanvas* canvas);
""")

__c_renderer = ffi.dlopen("libpyrender/librender.so")
#CAM_DIM = (1., .25)
#C_DIST_EFF = .25
#C_POS = np.array([0., 0., -25.])


def stl_forge(vertices, normals, scattering=0.95, refraction_index=15, flip_y=False):
    scene = Scene()
    if flip_y:
      vertices[:,:,1] *= -1
    for vertices, normal in zip(vertices, normals):
        add_triangle(
            scene,
            Triangle(vertices, normal, scattering, refraction_index)
        )
    return scene


def Vec3(vec):
    vec3 = ffi.new("PyVec3*")
    vec3.x, vec3.y, vec3.z = vec
    return vec3


def Light(loc, intensity):
    light = ffi.new("PyLight*")
    light.loc.x, light.loc.y, light.loc.z = loc
    light.intensity = intensity
    return light


def Triangle(vertices, normal, scattering=0.1, refraction_index=1.5):
    triangle = ffi.new("PyTriangle*")
    triangle.v0 = Vec3(vertices[0])[0]
    triangle.v1 = Vec3(vertices[1])[0]
    triangle.v2 = Vec3(vertices[2])[0]
    triangle.normal = Vec3(normal)[0]
    triangle.scattering = scattering
    triangle.refraction_index = refraction_index
    return triangle


def Scene():
    scene = ffi.new("PyScene*")
    __c_renderer.__init_scene(scene)
    #scene.add_triangle = lambda x: add_triangle(scene, x)
    #scene.add_light = lambda x: add_light(scene, x)
    #scene.render = lambda x: render(scene, x)
    return scene


def Canvas(width, height):
    canvas = ffi.new("PyCanvas*")
    __c_renderer.__init_canvas(canvas, width, height)
    return canvas


def add_triangle(scene, triangle):
    __c_renderer.add_triangle(triangle, scene)


def add_light(scene, light):
    __c_renderer.add_light(light, scene)


def render(scene, canvas):
    __c_renderer.render(scene, canvas)
    array = np.frombuffer(ffi.buffer(
        canvas.canvas, canvas.width*canvas.height*4), dtype=np.float32)
    return array.reshape(canvas.height, canvas.width)
