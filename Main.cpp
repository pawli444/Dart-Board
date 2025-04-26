#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderClass.h"
#include "Camera.h"

#include <vector>
#include <iostream>
#include <cmath>
#include "stb_image.h"

const double M_PI = 3.14159265358979323846;
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

float scale = 1.0f;

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        scale -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        scale += 0.01f;

    if (scale < 0.1f) scale = 0.1f;
    if (scale > 5.0f) scale = 5.0f;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Dartboard", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    Shader shader("default.vert", "default.frag");
    Camera camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 0.0f, 3.0f));

    const int segments = 64;
    const float thickness = 0.05f;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = cos(angle);
        float y = sin(angle);

        // Front face (z = thickness/2)
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(thickness / 2);
        vertices.push_back((x + 1.0f) / 2.0f); vertices.push_back((y + 1.0f) / 2.0f); // UV coords

        // Back face (z = -thickness/2)
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(-thickness / 2);
        vertices.push_back((x + 1.0f) / 2.0f); vertices.push_back((y + 1.0f) / 2.0f); // UV coords
    }

    int topCenterIndex = (int)(vertices.size() / 5);
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(thickness / 2);
    vertices.push_back(0.5f); vertices.push_back(0.5f);

    int bottomCenterIndex = (int)(vertices.size() / 5);
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(-thickness / 2);
    vertices.push_back(0.5f); vertices.push_back(0.5f);

    for (int i = 0; i < segments; i++) {
        int top1 = i * 2;
        int back1 = i * 2 + 1;
        int top2 = (i + 1) * 2;
        int back2 = (i + 1) * 2 + 1;

        // Side
        indices.push_back(top1); indices.push_back(back1); indices.push_back(back2);
        indices.push_back(top1); indices.push_back(back2); indices.push_back(top2);

        // Front
        indices.push_back(topCenterIndex); indices.push_back(top2); indices.push_back(top1);

        // Back
        indices.push_back(bottomCenterIndex); indices.push_back(back1); indices.push_back(back2);
    }

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("dartboard.png", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, nrChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture\n";
    }
    stbi_image_free(data);

    shader.Activate();
    shader.setInt("texture1", 0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processInput(window);
        camera.Inputs(window);

        glClearColor(0.85f, 0.85f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Activate();
        camera.Matrix(45.0f, 0.1f, 100.0f, shader, "cameraMatrix");

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Obrót wokó³ osi Y
        model = glm::scale(model, glm::vec3(scale));
        int modelLoc = glGetUniformLocation(shader.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &texture);

    shader.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
