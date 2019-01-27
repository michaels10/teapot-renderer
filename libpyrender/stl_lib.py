import numpy as np
import struct

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