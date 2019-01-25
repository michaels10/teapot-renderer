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
} PyTriangle;

typedef struct PyCanvas {
  int width;
  int height;
  float *canvas;
} PyCanvas;

typedef struct PyLight {
  PyVec3 loc;
  float intensity;
} PyLight;

void add_triangle(PyTriangle triangle);
void add_light(PyLight light);
PyCanvas render();
""")

__c_renderer = ffi.dlopen("./rtrtrender.so")
#CAM_DIM = (1., .25)
#C_DIST_EFF = .25
#C_POS = np.array([0., 0., -25.])


def stl_forge(vertices, normals):
    from tqdm import tqdm
    for np_triangle, np_normal in zip(tqdm(vertices), normals):
            vertices = [make_vec3(*v) for v in np_triangle]
            normal = make_vec3(*np_normal)
            args = vertices + [normal]
            add_triangle(make_triangle(*args))


def make_vec3(x, y, z):
    vec3 = ffi.new("PyVec3*")
    vec3.x = x
    vec3.y = y
    vec3.z = z
    return vec3


def make_light(loc, intensity):
    light = ffi.new("PyLight*")
    light.loc = loc[0]
    light.intensity = intensity
    return light


def make_triangle(v0, v1, v2, normal):
    triangle = ffi.new("PyTriangle*")
    triangle.v0 = v0[0]
    triangle.v1 = v1[0]
    triangle.v2 = v2[0]
    triangle.normal = normal[0]
    return triangle


def add_triangle(triangle):
    __c_renderer.add_triangle(triangle[0])


def add_light(light):
    __c_renderer.add_light(light[0])


def render():
    canvas = __c_renderer.render()
    array = np.frombuffer(ffi.buffer(
        canvas.canvas, canvas.width*canvas.height*4), dtype=np.float32)
    return array.reshape(canvas.height, canvas.width)
