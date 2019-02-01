#ifndef OCTREE_H
#define OCTREE_H
#include "render.h"
#include <vector>
using std::vector;

class OctreeNode;
struct BoundingBox;
struct Triangle;
struct Scene;

class Octree {
  public:
    OctreeNode *root;
    vector<Triangle *> &get_all_triangles(const Vec3 &point) const;
    OctreeNode *get_lowest_common_ancestor(OctreeNode *node1, OctreeNode *node2) const;
    OctreeNode *get_node(const Vec3 &point);
    Octree(const Scene &scene);
    Octree(const Octree &other);
    ~Octree();
};

class OctreeNode {
  private:
    OctreeNode *children[8];
    float yz_plane;
    float xz_plane;
    float xy_plane;

  public:
    int depth = 0;
    bool is_leaf = true;
    OctreeNode *parent = nullptr;
    OctreeNode *get_child(const Vec3 &point);
    OctreeNode(float yz, float xz, float xy) : yz_plane(yz), xz_plane(xz), xy_plane(xy){};
    OctreeNode(float yz, float xz, float xy, OctreeNode *parent)
        : yz_plane(yz), xz_plane(xz), xy_plane(xy), parent(parent) {
        depth = parent->depth + 1;
    };
    ~OctreeNode();
    void expand_children(float node_size, int depth, int max_depth);
    vector<const Triangle *> triangles;
};

struct BoundingBox {
  public:
    Vec3 min_xyz;
    Vec3 max_xyz;
    vector<Vec3> get_corners();
};

#endif