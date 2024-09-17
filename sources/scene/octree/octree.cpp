#include "octree.h"

#include <algorithm>


OctreeNode::OctreeNode(const AABB& volume) : _volume(volume), _children{} {

}

bool OctreeNode::addObject(const Object* object) {
    const glm::vec3& lc = _volume.lowerCorner;
    const glm::vec3& uc = _volume.upperCorner;

    glm::vec3 md = 0.5f * (lc + uc);

    const std::array<AABB, NUM_OCTREE_NODE_CHILDREN> subVolumes = {
        AABB{{lc.x, lc.y, lc.z}, {md.x, md.y, md.z}}, // Child 0: lower-left-front
        AABB{{md.x, lc.y, lc.z}, {uc.x, md.y, md.z}}, // Child 1: lower-right-front
        AABB{{lc.x, md.y, lc.z}, {md.x, uc.y, md.z}}, // Child 2: upper-left-front
        AABB{{md.x, md.y, lc.z}, {uc.x, uc.y, md.z}}, // Child 3: upper-right-front
        AABB{{lc.x, lc.y, md.z}, {md.x, md.y, uc.z}}, // Child 4: lower-left-back
        AABB{{md.x, lc.y, md.z}, {uc.x, md.y, uc.z}}, // Child 5: lower-right-back
        AABB{{lc.x, md.y, md.z}, {md.x, uc.y, uc.z}}, // Child 6: upper-left-back
        AABB{{md.x, md.y, md.z}, {uc.x, uc.y, uc.z}}  // Child 7: upper-right-back
    };

    auto index = std::find_if(subVolumes.cbegin(), subVolumes.cend(),
        [object](const AABB& subVolume) {
            return subVolume.contains(object->volume);
        }) - subVolumes.cbegin();

    if (index == NUM_OCTREE_NODE_CHILDREN) {
        _objects.push_back(object);
        return true;
    }
    else {
        if(!_children[index])
            _children[index] = std::make_unique<OctreeNode>(subVolumes[index]);
        return _children[index]->addObject(object);
    }
}

const OctreeNode* OctreeNode::getChild(Subvolume subvolume) const {
    return _children[static_cast<size_t>(subvolume)].get();
}

const AABB& OctreeNode::getVolume() const {
    return _volume;
}

const OctreeNode* OctreeNode::getChildIfVisible(Subvolume subvolume, const glm::mat4& VPmat) const {
    const OctreeNode* child = getChild(subvolume);
    if (child) {
        std::array<glm::vec4, NUM_CUBE_FACES> frustumPlanes = extractFrustumPlanes(VPmat);

        if (child->_volume.intersectsFrustum(frustumPlanes)) {
            return child;
        }
    }
    return nullptr;
}

const std::vector<const Object*>& OctreeNode::getObjects() const {
    return _objects;
}

Octree::Octree(const AABB& volume) : _root(std::make_unique<OctreeNode>(volume)) {

}

bool Octree::addObject(const Object* object) {
    return _root->addObject(object);
}

OctreeNode* Octree::getRoot() {
    return _root.get();
}

OctreeNode* Octree::getRootIfVisible(const glm::mat4& VPmat) {
    if (_root) {
        std::array<glm::vec4, NUM_CUBE_FACES> frustumPlanes = extractFrustumPlanes(VPmat);

        if (_root->_volume.intersectsFrustum(frustumPlanes)) {
            return _root.get();
        }
    }
    return nullptr;
}
