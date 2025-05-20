#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>

class Dart {
public:
    glm::vec3 position;
    glm::vec3 rotationAxis;
    float rotationAngle;
    float speed;
    bool hasHit;

    Dart();
    void Init();
    void Update(float deltaTime);
    void Draw(unsigned int shaderID);

private:
    unsigned int VAO, VBO;
};