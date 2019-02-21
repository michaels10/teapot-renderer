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

void OctreeNode::expand_children(int max_depth) {
    if (depth >= max_depth) {
        return;
    }
    is_leaf = false;
    for (unsigned bitcode = 0; bitcode < 8; bitcode++) {
        int xp = 2 * (bitcode >> 2) - 1;
        int yp = 2 * ((bitcode >> 1) & 1) - 1;
        int zp = 2 * (bitcode & 1) - 1;
        float yz_offset = xp * radial / 2;
        float xz_offset = yp * radial / 2;
        float xy_offset = zp * radial / 2;
        children[bitcode] = new OctreeNode(yz_plane + yz_offset, xz_plane + xz_offset, xy_plane * xy_offset, this);
        children[bitcode]->expand_children(max_depth);
    }
}

float OctreeNode::planes_intersection(const Vec3 &origin, const Vec3 &ray) {
    Vec3 inv_ray = 1.f / ray;
    float x_min = yz_plane - radial;
    float x_max = yz_plane + radial;
    float y_min = xz_plane - radial;
    float y_max = xz_plane + radial;
    float z_min = xy_plane - radial;
    float z_max = xy_plane + radial;
    float t_x_min, t_x_max, t_y_min, t_y_max, t_z_min, t_z_max, t_min, t_max;

    if (inv_ray.x >= 0) {
        t_x_min = (x_min - origin.x) / ray.x;
        t_x_max = (x_max - origin.x) / ray.x;
    } else {
        t_x_max = (x_min - origin.x) / ray.x;
        t_x_min = (x_max - origin.x) / ray.x;
    }

    t_min = t_x_min;
    t_max = t_x_max;

    if (inv_ray.y >= 0) {
        t_y_min = (y_min - origin.y) / ray.y;
        t_y_max = (y_max - origin.y) / ray.y;
    } else {
        t_y_max = (y_min - origin.y) / ray.y;
        t_y_min = (y_max - origin.y) / ray.y;
    }

    if ((t_min > t_y_max) || (t_y_min > t_max))
        return -1;
    if (t_y_min > t_min)
        t_min = t_y_min;
    if (t_y_max < t_max)
        t_max = t_y_max;

    if (inv_ray.z >= 0) {
        t_z_min = (z_min - origin.z) / ray.z;
        t_z_max = (z_max - origin.z) / ray.z;
    } else {
        t_z_max = (z_min - origin.z) / ray.z;
        t_z_min = (z_max - origin.z) / ray.z;
    }

    if ((t_min > t_z_max) || (t_min > t_z_max))
        return -1;
    if (t_z_min > t_min)
        t_min = t_z_min;
    if (t_z_max < t_max)
        t_max = t_z_max;

    return max((t_max + t_min) / 2.f, 0.f);
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
    while (a != b) {
        if (a->depth == b->depth) {
            a = a->parent;
            b = b->parent;
        } else if (a->depth > b->depth) {
            a = a->parent;
        } else {
            b = b->parent;
        }
    }
    return a;
}

Octree::Octree(const Scene &scene) {
    root = new OctreeNode(0.1, 0.1, 0.1, 7); // TODO: compute this from the scene.
    root->expand_children(8);
    printf("OCTN SZ: %zu\n", sizeof(OctreeNode) * 8 * 8 * 8 * 8 * 8 * 8);
    for (int i = 0; i < scene.geometry.size(); i++) {
        OctreeNode *root = nullptr;
        Triangle const *tri = &(scene.geometry[i]);
        BoundingBox box = tri->get_bounds();
        for (Vec3 &vec : box.get_corners()) {
            OctreeNode *node = get_node(vec);
            root = get_lowest_common_ancestor(root, node);
        }
        if (root->triangles == nullptr) {
            root->triangles = new vector<const Triangle *>();
        }
        root->triangles->push_back(tri);
    }
}
Octree::~Octree() { delete root; }
OctreeNode *Octree::get_node(const Vec3 &vec) const {
    OctreeNode *node = root;
    while (!node->is_leaf) {
        node = node->get_child(vec);
    }
    return node;
}

bool Octree::in_bounds(const Vec3 &vec) const {
    float x_dist = fabs(vec.x - root->yz_plane);
    float y_dist = fabs(vec.y - root->xz_plane);
    float z_dist = fabs(vec.z - root->xy_plane);
    return (x_dist <= root->radial) && (y_dist <= root->radial) && (z_dist <= root->radial);
}

OctreeLookup Octree::get_new_triangles(const Vec3 &point) const {
    OctreeLookup lookup;
    if (!in_bounds(point)) {
        return lookup;
    }
    OctreeNode *node = root;
    while (!node->is_leaf) {
        if (node->triangles != nullptr) {
            lookup.path.push_back(node->triangles);
        }
        node = node->get_child(point);
    }
    if (node->triangles != nullptr) {
        lookup.path.push_back(node->triangles);
    }
    lookup.node = node;
    lookup.is_valid = true;
    return lookup;
}

OctreeLookup Octree::get_new_triangles(const Vec3 &point, OctreeNode *previous_node) const {
    OctreeLookup lookup;
    if (!in_bounds(point)) {
        return lookup;
    }
    OctreeNode *lca = get_lowest_common_ancestor(previous_node, get_node(point));
    if (lca->is_leaf) {
        lookup.node = previous_node;
        lookup.is_valid = true;
        return lookup;
    }
    OctreeNode *node = lca->get_child(point);
    while (!node->is_leaf) {
        if (node->triangles != nullptr) {
            lookup.path.push_back(node->triangles);
        }
        node = node->get_child(point);
    }
    if (node->triangles != nullptr) {
        lookup.path.push_back(node->triangles);
    }
    lookup.node = node;
    lookup.is_valid = true;
    return lookup;
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