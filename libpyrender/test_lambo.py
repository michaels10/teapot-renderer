from matplotlib import pyplot as plt
import numpy as np
from model_lib import read_stl, read_obj
from render import stl_forge, Canvas, Light, add_light, render


vertices, normals = read_stl("stl/Lamborghini_Aventador.stl")
#scene = stl_forge(vertices, normals, flip_y=True, scattering=1.0)
middle = vertices.mean(0).mean(0)
vertices -= middle
scene = stl_forge(vertices+np.array([-5, 0, 3]), normals, scattering=1, refraction_index=1.8, flip_y=True)
add_light(scene, Light([0,10,-10], 1000))
canvas = Canvas(300, 300)
m = render(scene, canvas)
print(m)
print(m.min(), m.max())
plt.imsave("images/lambo.png", m, cmap='gist_gray')
