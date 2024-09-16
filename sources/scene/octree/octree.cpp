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

void extractFrustumPlanes(const glm::mat4& VP, glm::vec4 planes[6]) {
    planes[0] = glm::vec4(VP[0][3] + VP[0][0], VP[1][3] + VP[1][0], VP[2][3] + VP[2][0], VP[3][3] + VP[3][0]);
    planes[1] = glm::vec4(VP[0][3] - VP[0][0], VP[1][3] - VP[1][0], VP[2][3] - VP[2][0], VP[3][3] - VP[3][0]);
    planes[2] = glm::vec4(VP[0][3] + VP[0][1], VP[1][3] + VP[1][1], VP[2][3] + VP[2][1], VP[3][3] + VP[3][1]);
    planes[3] = glm::vec4(VP[0][3] - VP[0][1], VP[1][3] - VP[1][1], VP[2][3] - VP[2][1], VP[3][3] - VP[3][1]);
    planes[4] = glm::vec4(VP[0][3] + VP[0][2], VP[1][3] + VP[1][2], VP[2][3] + VP[2][2], VP[3][3] + VP[3][2]);
    planes[5] = glm::vec4(VP[0][3] - VP[0][2], VP[1][3] - VP[1][2], VP[2][3] - VP[2][2], VP[3][3] - VP[3][2]);

    for (int i = 0; i < 6; ++i) {
        float length = glm::length(glm::vec3(planes[i]));
        planes[i] /= length;
    }
}

bool isAABBVisible(const glm::vec3& lowerCorner, const glm::vec3& upperCorner, const glm::vec4 planes[6]) {
    for (int i = 0; i < 6; ++i) {
        const glm::vec4& plane = planes[i];
        glm::vec3 normal(plane.x, plane.y, plane.z);

        glm::vec3 positiveVertex = glm::vec3(
            (plane.x >= 0.0f) ? upperCorner.x : lowerCorner.x,
            (plane.y >= 0.0f) ? upperCorner.y : lowerCorner.y,
            (plane.z >= 0.0f) ? upperCorner.z : lowerCorner.z
        );

        if (glm::dot(normal, positiveVertex) + plane.w < 0.0f) {
            return false; 
        }
    }

    return true;
}

const OctreeNode* OctreeNode::getChildIfVisible(Subvolume subvolume, const glm::mat4& VPmat) const {
    const OctreeNode* child = getChild(subvolume);
    if (child) {
        glm::vec4 frustumPlanes[6];
        extractFrustumPlanes(VPmat, frustumPlanes);

        const glm::vec3& lowerCorner = child->_volume.lowerCorner;
        const glm::vec3& upperCorner = child->_volume.upperCorner;

        if (isAABBVisible(lowerCorner, upperCorner, frustumPlanes)) {
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
        glm::vec4 frustumPlanes[6];
        extractFrustumPlanes(VPmat, frustumPlanes);

        const glm::vec3& lowerCorner = _root->_volume.lowerCorner;
        const glm::vec3& upperCorner = _root->_volume.upperCorner;

        if (isAABBVisible(lowerCorner, upperCorner, frustumPlanes)) {
            return _root.get();
        }
    }
    return nullptr;
}
