from matplotlib import pyplot as plt
import numpy as np
from model_lib import read_stl
from render import stl_forge, Canvas, Light, add_light, render


vertices, normals = read_stl("stl/teacup-plane.stl")
scene = stl_forge(vertices, normals, flip_y=True, scattering=0.7)
add_light(scene, Light([0,10,-10], 1000))
canvas = Canvas(1000, 1000)
m = render(scene, canvas)

plt.imsave("images/plane_teacup_front.png", m, cmap='gist_gray')