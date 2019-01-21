import numpy as np
import scipy as sp
import scipy.linalg as la
import curses, struct, logging
import cProfile


DENSITIES = [" ", "\u2591", "\u2592", "\u2593", "\u2588"]

################ GENERATE CUBE #########################

def triangulate_quad(coords):
    tri1 = np.array([coords[0], coords[1], coords[2]])
    tri2 = np.array([coords[2], coords[3], coords[0]])
    return tri1, tri2

# specified clockwise order 
def quad_2d(vert, vec1, vec2):
    verts = [vert, vert + vec1, vert + vec1 + vec2, vert + vec2]
    return triangulate_quad(np.array(verts))

def quad_volume(main_vert, vec1, vec2, vec3):
    """this generates a paralelliped.
    Note that this is mostly used for debugging."""
    # cover all faces including main_vert
    face_1 = quad_2d(main_vert, vec1, vec2)
    face_2 = quad_2d(main_vert, vec2, vec3)
    face_3 = quad_2d(main_vert, vec3, vec1)

    # do all faces not including main_vert
    opp_vert = main_vert + vec1 + vec2 + vec3
    face_4 = quad_2d(opp_vert, -vec3, -vec2)
    face_5 = quad_2d(opp_vert, -vec2, -vec1)
    face_6 = quad_2d(opp_vert, -vec1, -vec3)

    return face_1 + face_2 + face_3 + face_4 + face_5 + face_6

################ READ IN STL ############################

def read_stl(f_name):
    """ Reads in an arbitrary STL file.
    TODO: Large model support"""

    # this relies on SMALL models
    in_file = open(f_name, "rb")
    bytes_read = in_file.read()
    in_file.close()

    tris = []
    normals = []
    get_f_str = lambda i: "".str(bytes_read[i:i+4])
    read_vec = lambda i: np.array(struct.unpack('f'*3, bytes_read[i: i + 12]))

    # skip first 80 bytes, and skip number of triangles
    # first 4 is # of arrays, second is # of points in a vec,
    # third is # of bytes in a vec
    # each iteration, skip 4*3*4 + 2 bytes
    for i in range(84, len(bytes_read), 4*(3*4) + 2):
        v0 = read_vec(i + 0*(3*4)) 
        v0[1], v0[2] = -v0[2], v0[1]

        # read in vertices
        v1 = read_vec(i + 1*(3*4))
        v1[1], v1[2] = -v1[2], v1[1]
        v2 = read_vec(i + 2*(3*4))
        v2[1], v2[2] = -v2[2], v2[1]
        v3 = read_vec(i + 3*(3*4))
        v3[1], v3[2] = -v3[2], v3[1]

        normals.append(np.array(v0))
        tris.append(np.array([v1, v2, v3]))
    return np.array(tuple(tris)), np.array(tuple(normals))

########################### RENDERING CODE ##############################


# this is the model we're supposed to render
MODEL, MODEL_NORMALS = read_stl("UtahTeapot.stl")
# MODEL = np.array(quad_volume(np.array([0, 0, 10]), np.array([3, 0, 0]), np.array([0, 3, 0]), np.array([0, 0, 3])))
LIGHT_DIR = np.array([1, 1, 0]) 
LIGHT_DIR = LIGHT_DIR / np.linalg.norm(LIGHT_DIR)

CAM_DIM = (1., .25)
C_DIST_EFF = .25
C_POS = np.array([0., 0., -25.])

def render_object(c_pos, p_pos, shift_tris, normals):
    exists_collision = False 
    cur_tri = None
    cur_distance = 1000000
    
    for i, shift_tri in enumerate(shift_tris):
        # want to solve system of equations:
        # [p_pos - c_pos, v1 - v2, v2 - v3] [t a b]^t = [v1 - cpos]
        cur_matrix = np.vstack((p_pos, shift_tri[1], shift_tri[2])).T
        try:
            solved = la.solve(cur_matrix, shift_tri[0])
        except la.LinAlgError:
            continue
        # make sure all elements greater than .01, and that we're inside the triangle!
        # true if collides, false if no
        collides = np.logical_and(np.all(solved >= 0.0001), np.all(solved[1:] <= 1))
        # as long as p_pos is constant, scales linearly
        distance = solved[0]

        exists_collision = np.logical_or(exists_collision, collides)
            
        # keep track of the closest tri
        if collides and distance < cur_distance:
            cur_tri, cur_distance = i, distance

    return do_lighting(normals[i]) if exists_collision else DENSITIES[0]


def do_lighting(tri):
    # get surface normal
    surface_normal = -np.cross(tri[1] - tri[0], tri[2] - tri[0])
    surface_normal = surface_normal / np.linalg.norm(surface_normal)

    # magic magic to convert surface normal to lighting
    parallel_measure = np.inner(surface_normal, LIGHT_DIR)
    parallel_measure *= .5
    parallel_measure += .5
    parallel_measure *= 3.99

    return DENSITIES[int(parallel_measure) + 1] 


######################## DISPLAY ########################

STDSCR = curses.initscr()
PIXEL_ASPECT = 8./17
RESOLUTION = (160, 40)

def process_input(char):
    forwards_vec = np.array([0., 0., 1.])
    side_vec = np.array([1., 0., 0.])
    up_vec = np.array([0., 1., 0])

    # return 1 if char == x, -1 if char == y, 0 otherwise.
    pick_one = lambda char, x, y: int(char == x) - int(char == y)

    net_movement = pick_one(char, "w", "s") * forwards_vec
    net_movement += pick_one(char, "d", "a") * side_vec 
    net_movement += pick_one(char, "f", "r") * up_vec

    return net_movement


# SUPER suboptimal
def update_window_model(window, model, model_normals):
    window.clear()
    dims = window.getmaxyx()
    xs = np.arange(-dims[1]//2 + 1, dims[1] // 2) * CAM_DIM[0] / float(dims[1]) * PIXEL_ASPECT
    ys = np.arange(-dims[0]//2 + 1, dims[0] // 2) * CAM_DIM[1] / float(dims[0]) 

    # select the nth vertex slice
    vcol0 = model[:, 0, :]
    vcol1 = model[:, 1, :]
    vcol2 = model[:, 2, :]
    repeated_c_pos = np.tile(C_POS, (len(model), 1))

    sub_matrix = np.hstack((repeated_c_pos, vcol1, vcol2)).reshape(len(model), 3, 3)
    add_matrix = np.hstack((vcol0, vcol0, vcol0)).reshape(len(model), 3, 3)
    # doing this now saves lots of ops; is sort of a general purpose utility array
    shifted_tris = add_matrix - sub_matrix 

    for y, true_y in enumerate(ys):
        for x, true_x in enumerate(xs):
            # pixel position relative to camera position
            p_pos = np.array([true_x, true_y, C_DIST_EFF])
            char = render_object(C_POS, p_pos, shifted_tris, model_normals)
            window.addch(y, x, char)
            window.refresh()
    window.refresh()

def main():
    global C_POS
    while True:
        update_window_model(STDSCR, MODEL, MODEL_NORMALS)
        key = STDSCR.getkey()
        C_POS += process_input(key)

curses.wrapper(main())
