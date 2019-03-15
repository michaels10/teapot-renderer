import itertools as it
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

fig = plt.figure()

box_points = ((0, 0, 0), (1, 1, 1))
U = np.zeros((20, 20, 20))

def get_res():
    res_x = box_points[1][0] / float(len(U))
    res_y = box_points[1][1] / float(len(U))
    res_z = box_points[1][2] / float(len(U))

    return res_x, res_y, res_z

res = get_res()

ax = fig.add_subplot(111, projection = '3d')

n_x = 20
n_y = 20
n_z = 10

x = np.linspace(0, 1, n_x)
y = np.linspace(0, 1, n_y)
z = np.linspace(0, .5, n_z)

points = np.array(list(it.product(x, y, z)))

xs = points[:, 0]
ys = points[:, 1]
zs = points[:, 2]

ax.scatter(xs, ys, zs)
ax.set_zlim(0, 1)

plt.show()
