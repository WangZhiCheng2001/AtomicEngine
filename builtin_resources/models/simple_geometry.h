#pragma once

#define _USE_MATH_DEFINES
#include <cstdint>
#include <cmath>
#include <vector>
#include <tuple>

#include <glm/glm.hpp>

struct VertexAttribute
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

inline auto generateTestQuad()
    -> std::pair<std::vector<float>, std::vector<uint32_t>>
{
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    vertices.insert(vertices.end(), {-.5f, -.5f, .0f, .0f, .0f, -1.f, .0f, .0f});
    vertices.insert(vertices.end(), {.5f, .5f, .0f, .0f, .0f, -1.f, 1.f, 1.f});
    vertices.insert(vertices.end(), {-.5f, .5f, .0f, .0f, .0f, -1.f, .0f, 1.f});
    vertices.insert(vertices.end(), {.5f, -.5f, .0f, .0f, .0f, -1.f, 1.f, .0f});
    indices.insert(indices.end(), {0, 2, 1});
    indices.insert(indices.end(), {1, 3, 0});
    return {vertices, indices};
}

inline auto generateSphere(const float &radius, const uint32_t &segmentX, const uint32_t &segmentY)
    -> std::pair<std::vector<float>, std::vector<uint32_t>>
{
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    for (auto y = 0; y <= segmentY; ++y)
    {
        for (auto x = 0; x <= segmentX; ++x)
        {
            auto coeffX = static_cast<float>(x) / segmentX;
            auto coeffY = static_cast<float>(y) / segmentY;
            float posX = radius * std::cos(M_PI * 0.5 - M_PI * coeffY) * std::cos(2 * M_PI * coeffX);
            float posY = radius * std::cos(M_PI * 0.5 - M_PI * coeffY) * std::sin(2 * M_PI * coeffX);
            float posZ = radius * std::sin(M_PI * 0.5 - M_PI * coeffY);

            vertices.insert(vertices.end(), {posX, posY, 0.f, posX / radius, posY / radius, 0 / radius, coeffX, coeffY});
        }
    }
    for (auto y = 0; y < segmentY; ++y)
    {
        for (auto x = 0; x < segmentX; ++x)
        {
            if (y != 0)
                indices.insert(indices.end(), {y * (segmentX + 1), y * (segmentX + 1) + segmentX + 1, y * (segmentX + 1) + 1});
            if (y != (segmentY - 1))
                indices.insert(indices.end(), {y * (segmentX + 1) + 1, y * (segmentX + 1) + segmentX + 1, y * (segmentX + 1) + segmentX + 2});
        }
    }
    return std::pair{vertices, indices};
}