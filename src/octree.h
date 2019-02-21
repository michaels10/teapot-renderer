#ifndef OCTREE_H
#define OCTREE_H
#include "render.h"
#include <vector>
using std::vector;

class OctreeNode;
struct OctreeLookup;
struct BoundingBox;
struct Triangle;
struct Scene;

class Octree {
  public:
    OctreeNode *root;
    vector<Triangle *> &get_all_triangles(const Vec3 &point) const;
    OctreeNode *get_lowest_common_ancestor(OctreeNode *node1, OctreeNode *node2) const;
    OctreeLookup get_new_triangles(const Vec3 &point) const;
    OctreeLookup get_new_triangles(const Vec3 &point, OctreeNode *previous_node) const;
    OctreeNode *get_node(const Vec3 &point) const;
    Octree(const Scene &scene);
    Octree(const Octree &other);
    bool in_bounds(const Vec3 &point) const;
    ~Octree();
};

class OctreeNode {
  public:
    bool is_leaf : 1 = true;
    int depth : 31 = 0;
    OctreeNode *children[8];
    float yz_plane;
    float xz_plane;
    float xy_plane;
    float radial;
    OctreeNode *parent = nullptr;
    OctreeNode *get_child(const Vec3 &point);
    OctreeNode(float yz, float xz, float xy, float radial) : yz_plane(yz), xz_plane(xz), xy_plane(xy), radial(radial){};
    OctreeNode(float yz, float xz, float xy, OctreeNode *parent)
        : yz_plane(yz), xz_plane(xz), xy_plane(xy), parent(parent) {
        radial = parent->radial / 2;
        depth = parent->depth + 1;
    };
    ~OctreeNode();
    void expand_children(int max_depth);
    float planes_intersection(const Vec3 &origin, const Vec3 &ray);
    vector<const Triangle *> *triangles = nullptr;
};

struct OctreeLookup {
    bool is_valid = false;
    OctreeNode *node = nullptr;
    vector<vector<const Triangle *> *> path;
};

struct BoundingBox {
  public:
    Vec3 min_xyz;
    Vec3 max_xyz;
    vector<Vec3> get_corners();
};

#endif