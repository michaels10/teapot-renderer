from matplotlib import pyplot as plt
import numpy as np
from stl_lib import read_stl
from render import stl_forge, Canvas, Light, add_light, render


#vertices, normals = read_stl("stl/teacup-plane.stl")
#scene = stl_forge(vertices, normals, flip_y=True, scattering=1.0)
vertices, normals = read_stl("stl/UtahTeapot.stl")
middle = vertices.mean(0).mean(0)
vertices -= middle
scene = stl_forge(vertices+np.array([-5, 0, 3]), normals, scattering=.5, refraction_index=1.333, flip_y=True)
add_light(scene, Light([0,10,-10], 1000))
canvas = Canvas(300, 300)
m = render(scene, canvas)

plt.imsave("images/void_teapot_frosted_front.png", m, cmap='gist_gray')
