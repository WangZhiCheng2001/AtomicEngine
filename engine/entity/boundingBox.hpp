#pragma once

#include <glm/glm.hpp>

struct BoundingBox
{
    auto getCenter()
    {
        return (minPoint + maxPoint) * .5f;
    }
    auto getExtent()
    {
        return maxPoint - minPoint;
    }
    auto  getHalfExtent()
    {
        return getExtent() * .5f;
    }

    void extend(const glm::vec3 &p)
    {
        minPoint = glm::min(minPoint, p);
        maxPoint = glm::max(maxPoint, p);
    }
    void extend(const BoundingBox &box)
    {
        minPoint = glm::min(minPoint, box.minPoint);
        maxPoint = glm::max(maxPoint, box.maxPoint);
    }

    glm::vec3 minPoint{glm::zero<glm::vec3>()};
    glm::vec3 maxPoint{glm::zero<glm::vec3>()};
};