from cffi import FFI
import numpy as np

ffi = FFI()

ffi.cdef("""
typedef struct PyVec3 {
  double x, y, z;
} PyVec3;

typedef struct PyLight {
  PyVec3 loc;
  float intensity;
} PyLight;

typedef struct PyCanvas {
  int width;
  int height;
  float *canvas;
  void* cpp_canvas;
} PyCanvas;

typedef struct PyScene {
  void* scene;
} PyScene;

void add_mesh(PyScene * scene, float *vert, size_t vert_len, int *tris, size_t tris_len, float scttrng, float ior);
void add_light(PyScene *scene, PyLight *pylight);
void __init_scene(PyScene *scene);
void __init_canvas(PyCanvas *canvas, int width, int height);
void render(PyScene* scene, PyCanvas* canvas);
""")

__c_renderer = ffi.dlopen("libpyrender/librender.so")

### scene-level and related inits ###
def add_light(scene, light):
    __c_renderer.add_light(scene, light)

def Scene():
    scene = ffi.new("PyScene*")
    __c_renderer.__init_scene(scene)
    return scene

def Canvas(width, height):
    canvas = ffi.new("PyCanvas*")
    __c_renderer.__init_canvas(canvas, width, height)
    return canvas

def render(scene, canvas):
    __c_renderer.render(scene, canvas)
    buff = ffi.buffer(canvas.canvas, canvas.width*canvas.height*4)
    array = np.frombuffer(buff, dtype=np.float32)
    return array.reshape(canvas.height, canvas.width)

### light inits ###
def Light(loc, intensity):
    light = ffi.new("PyLight*")
    light.loc.x, light.loc.y, light.loc.z = loc
    light.intensity = intensity
    return light

### mesh inits ### 
def Mesh(scene, verts, tris, scattering=0.95, ior=15, flip_y=False):
    if flip_y: verts[:, 1] *= -1   
    print(len(verts))
    print(len(tris))
    vert_buf = ffi.cast("float *", ffi.from_buffer(verts.data))
    tri_buf = ffi.cast("int *", ffi.from_buffer(tris.data))
    __c_renderer.add_mesh(scene, vert_buf, len(verts), tri_buf, len(tris), scattering, ior)

