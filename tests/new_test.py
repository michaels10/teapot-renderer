from matplotlib import pyplot as plt
import numpy as np

# this is bad practice but sue me.
import sys
sys.path.insert(0, 'libpyrender/')
from model_lib import read_obj 
from render import Mesh, Canvas, Light, add_light, render, Scene


mesh_list = read_obj("models/teacup.obj")
scene = Scene()
for mesh in mesh_list:
    Mesh(scene, mesh.verts, mesh.faces, flip_y = False, scattering= .7)

add_light(scene, Light([0,10,-10], 1000))
canvas = Canvas(100, 100)
m = render(scene, canvas)

plt.imsave("images/plane_teacup_front.png", m, cmap='gist_gray')
