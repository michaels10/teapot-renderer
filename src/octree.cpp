#include "octree.h"

OctreeNode *OctreeNode::get_child(const Vec3 &point) {
    if (is_leaf) {
        return nullptr;
    }
    unsigned bitcode = 0;
    bitcode |= (point.x >= yz_plane) << 2;
    bitcode |= (point.y >= xz_plane) << 1;
    bitcode |= (point.z >= yz_plane) << 0;
    return children[bitcode];
}

void OctreeNode::expand_children(float node_size, int depth, int max_depth) {
    if (depth > max_depth) {
        return;
    }
    is_leaf = false;
    for (unsigned bitcode = 0; bitcode < 8; bitcode++) {
        int xp = 2 * (bitcode >> 2) - 1;
        int yp = 2 * ((bitcode >> 1) & 1) - 1;
        int zp = 2 * (bitcode & 1) - 1;
        float yz_offset = xp * node_size / 2;
        float xz_offset = yp * node_size / 2;
        float xy_offset = zp * node_size / 2;
        children[bitcode] = new OctreeNode(yz_plane + yz_offset, xz_plane + xz_offset, xy_plane * xy_offset, this);
        children[bitcode]->expand_children(node_size / 2, depth + 1, max_depth);
    }
}

OctreeNode::~OctreeNode() {
    if (!is_leaf) {
        for (int i = 0; i < 8; i++) {
            delete children[i];
        }
    }
}

OctreeNode *Octree::get_lowest_common_ancestor(OctreeNode *a, OctreeNode *b) const {
    if (a == nullptr) {
        return b;
    }
    if (b == nullptr) {
        return a;
    }
    while (a != b) { // lol so slow
        if (a->depth > b->depth) {
            a = a->parent;
        } else {
            b = b->parent;
        }
    }
    return a;
}

Octree::Octree(const Scene &scene) {
    float octree_dim = 20;          // The initial total side length of a cuboid    
    root = new OctreeNode(0, 0, 0); // The center - both of these should really be computed from the scene.
    root->expand_children(octree_dim / 2, 1, 5);

    for (const Mesh& mesh: scene.geometry) {
        for (int i = 0; i < mesh.tris.size(); i++) {
            OctreeNode *root = nullptr;
            RefTriangle const *ref_tri = &(mesh.tris[i]);
            BoundingBox box = mesh.get_triangle(*ref_tri).get_bounds();
            for (Vec3 &vec : box.get_corners()) {
                OctreeNode *node = get_node(vec);
                root = get_lowest_common_ancestor(root, node);
            }
            root->triangles.push_back(ref_tri);
	}
    }
}
Octree::~Octree() { delete root; }
OctreeNode *Octree::get_node(const Vec3 &vec) {
    OctreeNode *node = root;
    while (!node->is_leaf) {
        node = node->get_child(vec);
    }
    return node;
}

vector<Vec3> BoundingBox::get_corners() {
    vector<Vec3> rv(8);
    for (unsigned bitpattern = 0; bitpattern < 8; bitpattern++) {
        Vec3 vec;
        vec.x = bitpattern & 1 ? max_xyz.x : min_xyz.x;
        vec.y = (bitpattern >> 1) & 1 ? max_xyz.y : min_xyz.y;
        vec.z = (bitpattern >> 2) & 1 ? max_xyz.z : min_xyz.z;
        rv.push_back(vec);
    }
    return rv;
}
