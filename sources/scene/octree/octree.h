#pragma once

#include "object/volumes.h"
#include "object/object.h"

#include <array>
#include <vector>
#include <memory>

constexpr size_t NUM_OCTREE_NODE_CHILDREN = 8;

class OctreeNode {
    enum class Subvolume : size_t {
        LOWER_LEFT_FRONT = 0,
        LOWER_RIGHT_FRONT,
        UPPER_LEFT_FRONT,
        UPPER_RIGHT_FRONT,
        LOWER_LEFT_BACK,
        LOWER_RIGHT_BACK,
        UPPER_LEFT_BACK,
        UPPER_RIGHT_BACK
    };


	AABB _volume;
	std::array<std::unique_ptr<OctreeNode>, NUM_OCTREE_NODE_CHILDREN> _children;
	std::vector<const Object*> _objects;

public:
	OctreeNode(const AABB& volume);
	void addObject(const Object* object);
    OctreeNode* getChild(Subvolume subvolume);
    const std::vector<const Object*>& getObjects() const;
};

class Octree {
    std::unique_ptr<OctreeNode> _root;

public:
    Octree(const AABB& volume);
    Octree();

    void addObject(const Object* object);
    OctreeNode* getRoot();
};