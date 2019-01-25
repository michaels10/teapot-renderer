from matplotlib import pyplot as plt
from stl_lib import read_stl
from render import stl_forge, Canvas, Light, add_light, render


vertices, normals = read_stl("stl/teacup-plane.stl")
scene = stl_forge(vertices, normals)
add_light(scene, Light([0,0,-10], 10))
canvas = Canvas(600, 600)
m = render(scene, canvas)

plt.imsave("render.png", m, cmap='Greys')