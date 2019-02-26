import numpy as np
import struct
import render

class PyMesh:
    name = ""
    verts = []
    faces = []
    normals = []
    def __init__(self):
        self.verts = []
        self.faces = []
        # self.normals = []

def read_obj(f_name, ccw_winding=True):
    """
    Read an obj file.
    WILL READ INTO CW WINDING!
    """
    meshes = []
    lines = [] 
    with open(f_name, "r") as f:
        lines =f.readlines()

    for line in lines:
        line = line.strip() 
        if len(line) == 0 or line[0] == "#":
            continue
        _, *split_line = line.split()
        
        if line[0] == "o":
            meshes.append(PyMesh())
        elif line[0] == "v":
            meshes[-1].verts += [[float(coord) for coord in split_line]]
        elif line[0] == "f":
            split_line = split_line[::-1] if ccw_winding else split_line
            def get_vert(face): return int(face.split("/")[0])
            meshes[-1].faces += [[get_vert(face) - 1 for face in split_line]]

    for mesh in meshes:
        mesh.verts = np.array(mesh.verts, dtype = np.single)
        # mesh.normals = np.copy(mesh.verts)
        mesh.faces = np.array(mesh.faces, dtype = np.intc)
    print(len(meshes))
    return meshes 

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

    out_obj = MeshTriangular()
    index = 0
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

        out_obj.normals.append(np.array(v0))
        tris.append(([v1, v2, v3]))

        index += 3

    out_obj.tris = tris
    out_obj.normals = normals 
    return np.array(tuple(tris)), np.array(tuple(normals))

