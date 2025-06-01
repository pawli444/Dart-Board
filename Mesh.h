#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shaderClass.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec2 TexCoords;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
    void Draw(Shader& shader);

private:
    unsigned int VBO, EBO;
    void setupMesh();
};
