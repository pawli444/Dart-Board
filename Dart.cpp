#include "Dart.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Dart::Dart() {
    position = glm::vec3(0.0f, 0.0f, 2.0f); 
    rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f); 
    rotationAngle = 0.0f;
    speed = 1.5f;
    hasHit = false;
    VAO = 0;
    VBO = 0;
}

void Dart::Init() {
    float dartVertices[] = {
        // grot 
        0.0f,  0.0f, -0.2f,
       -0.03f, 0.03f, 0.0f,
        0.03f, 0.03f, 0.0f,

        // trzon 
       -0.01f, -0.01f, 0.0f,
        0.01f, -0.01f, 0.0f,
        0.01f, -0.01f, 0.3f,
       -0.01f, -0.01f, 0.3f,

       // lotka 
       0.0f, 0.0f, 0.3f,
      -0.04f, 0.0f, 0.35f,
       0.04f, 0.0f, 0.35f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dartVertices), dartVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void Dart::Update(float deltaTime) {
    if (!hasHit) {
        position.z -= speed * deltaTime;
        rotationAngle += 360.0f * deltaTime;
        if (position.z <= 0.25f) {
            hasHit = true;
        }
    }
}

void Dart::Draw(unsigned int shaderID) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);
    int modelLoc = glGetUniformLocation(shaderID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);

    // grot 
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // trzon 
    glDrawArrays(GL_TRIANGLE_FAN, 3, 4);

    // lotka 
    glDrawArrays(GL_TRIANGLES, 7, 3);
}
