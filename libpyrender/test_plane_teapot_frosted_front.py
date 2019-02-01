from matplotlib import pyplot as plt
import numpy as np
from render import stl_forge, Canvas, Light, Triangle, add_light, render, add_triangle, Triangle
from model_lib import read_stl


#vertices, normals = read_stl("stl/teacup-plane.stl")
#scene = stl_forge(vertices, normals, flip_y=True, scattering=1.0)
vertices, normals = read_stl("stl/UtahTeapot.stl")
middle = vertices.mean(0).mean(0)
vertices -= middle
scene = stl_forge(vertices+np.array([-5, 0, 3]), normals, scattering=.5, refraction_index=1.333, flip_y=True)
ground_plane_verts = np.array([
    [-100, -4, -100],
    [-100, -4, 100],
    [100, -4, 100],
    [100, -4, -100],
])
ground_plane_normal = np.array([0,1,0])
add_triangle(scene,
    Triangle(ground_plane_verts[[0,1,2]], ground_plane_normal, scattering=1.0)
)
add_triangle(scene,
    Triangle(ground_plane_verts[[0,3,2]], ground_plane_normal, scattering=1.0)
)

add_light(scene, Light([0,10,-10], 1000))
canvas = Canvas(1000, 1000)
m = render(scene, canvas)

plt.imsave("images/plane_teapot_frosted_front.png", m, cmap='gist_gray')
