from matplotlib import pyplot as plt
from stl_lib import read_stl
from render import stl_forge, Canvas, Light, add_light, render


vertices, normals = read_stl("stl/teacup-plane.stl")
scene = stl_forge(vertices, normals, flip_y=True)
add_light(scene, Light([0,10,-10], 1000))
canvas = Canvas(2048, 2048)
m = render(scene, canvas)

print(m)
print(m.min(), m.max())
plt.imsave("render.png", m, cmap='gist_gray')