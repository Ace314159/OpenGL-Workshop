#ifndef OPENGL_WORKSHOP_WORLDGEN_H
#define OPENGL_WORKSHOP_WORLDGEN_H

#include <ranges>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "World.h"

glm::vec3 faces[6][4] = {
        // Top
        {{1, 1, 0},
                {0, 1, 0},
                {0, 1, 1},
                {1, 1, 1}},
        // Front
        {{1, 1, 1},
                {0, 1, 1},
                {0, 0, 1},
                {1, 0, 1}},
        // Right
        {{1, 1, 0},
                {1, 1, 1},
                {1, 0, 1},
                {1, 0, 0}},
        // Back
        {{0, 1, 0},
                {1, 1, 0},
                {1, 0, 0},
                {0, 0, 0}},
        // Left
        {{0, 1, 1},
                {0, 1, 0},
                {0, 0, 0},
                {0, 0, 1}},
        // Bottom
        {{1, 0, 1},
                {0, 0, 1},
                {0, 0, 0},
                {1, 0, 0}},
};

struct Vertex {
    glm::vec4 position;
    glm::vec3 color;
};

constexpr glm::vec3 BROWN = {0.51, 0.329, 0.196};
constexpr glm::vec3 GREEN = {0.365, 0.486, 0.082};
const glm::mat4 projMat = glm::perspective(glm::radians(45.0), 800.0 / 600.0,
                                           0.1, 2000.0);
const glm::vec3 pos{-129.906143, 110.205849, 3.054655};
const glm::mat4 viewMat = glm::lookAt(pos, {-130.774033, 109.952087, 3.481709}, {0.000000, 1.000000, 0.000000});

std::pair<std::vector<Vertex>, std::vector<GLuint>> makeVertexData() {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    auto filtered = heightMap | std::views::filter([](const auto &heightData) {
        const auto &[x, height, z] = heightData;
        float xFloat = x, zFloat = z;
        return pos.x - 90 < xFloat && xFloat < pos.x - 10 && pos.z - 10 < zFloat && zFloat < pos.z + 70;
    });
    for (const auto &[x, height, z]: filtered) {
        for (int y = 65; y < height; ++y) {
            bool top = true;
            for (const auto &face: faces) {
                const glm::vec3 &color = (top && y == height - 1) ? GREEN : BROWN;
                top = false;

                GLuint faceStartI = vertices.size();
                indices.push_back(faceStartI + 0);
                indices.push_back(faceStartI + 1);
                indices.push_back(faceStartI + 2);
                indices.push_back(faceStartI + 0);
                indices.push_back(faceStartI + 2);
                indices.push_back(faceStartI + 3);
                for (auto vecPos: face) {
                    vecPos += glm::vec3{x, y, z};
                    vertices.push_back({glm::vec4{projMat * viewMat * glm::vec4(vecPos, 1)}, color});
                }
            }
        }
    }
    return {vertices, indices};
}

#endif //OPENGL_WORKSHOP_WORLDGEN_H
