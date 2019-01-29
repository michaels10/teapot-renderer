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

    def get_f_str(i): return "".str(bytes_read[i:i+4])

    def read_vec(i): return np.array(
        struct.unpack('f'*3, bytes_read[i: i + 12]))

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


def read_obj(f_name, ccw_winding=True):
    """
    Read an obj file.
    WILL READ INTO CW WINDING!
    """
    verts = []
    faces = []

    def add_vertex(line):
        nonlocal verts
        _, *xyz = line.split()
        verts += [list(map(float, xyz))]

    def add_face(face):
        nonlocal faces, ccw_winding
        _, *ind = line.split()
        if ccw_winding:
            ind = ind[::-1]
        face_indices = []
        for i, face in enumerate(ind):
            fi, *_ = face.split("/")
            face_indices.append(fi)
        faces += [[int(x)-1 for x in face_indices]]

    with open(f_name, "r") as f:
        for line in f:
            line = line.strip()
            if len(line) == 0 or line[0] == "#":
                continue
            {
                "v": add_vertex,
                "f": add_face
            }.get(line[0], lambda x: None)(line)

    tris = []
    for face in faces:
        tri = []
        for ind in face:
            tri += [verts[ind]]
        tris += [tri]
    return np.array(tris, dtype=np.float16)