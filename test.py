from matplotlib import pyplot as plt
from stl_lib import read_stl
from render_ffi import stl_forge, render, make_vec3, make_light, add_light, add_triangle, make_triangle


vertices, normals = read_stl("UtahTeapot.stl")
stl_forge(vertices, normals)


#v0 = make_vec3(0, -1, 1)
#v1 = make_vec3(-1, 0, 1)
#v2 = make_vec3(1, 0, 1)
#n = make_vec3(0, 0, -1)
#add_triangle(make_triangle(v0, v1, v2,n))
add_light(make_light(make_vec3(0, 0, -10), 10))

m = render()
print(m)
plt.imsave("render.png", m)