#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderClass.h"
#include "Camera.h"
#include "Dart.h"
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <cmath>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
const double M_PI = 3.14159265358979323846;

Camera camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 0.0f, 3.0f));
float scale = 1.0f;

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        scale -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        scale += 0.01f;
    if (scale < 0.1f) scale = 0.1f;
    if (scale > 5.0f) scale = 5.0f;
}

unsigned int loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

int main()
{
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
    shader.Activate();
    shader.setInt("texture1", 0);

    // Load textures
    unsigned int wallTexture = loadTexture("wall.png");
    unsigned int dartboardTexture = loadTexture("dartboard.png");

    // Wall geometry
    float wallVertices[] = {
        -2.5f,  2.5f,  0.0f,  0.0f, 1.0f,
        -2.5f, -2.5f,  0.0f,  0.0f, 0.0f,
         2.5f, -2.5f,  0.0f,  1.0f, 0.0f,
         2.5f,  2.5f,  0.0f,  1.0f, 1.0f
    };
    unsigned int wallIndices[] = { 0, 1, 2, 0, 2, 3 };
    unsigned int wallVAO, wallVBO, wallEBO;
    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);
    glGenBuffers(1, &wallEBO);
    glBindVertexArray(wallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), wallVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(wallIndices), wallIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Dartboard geometry
    const int segments = 64;
    const float thickness = 0.05f;
    std::vector<float> dartboardVertices;
    std::vector<unsigned int> dartboardIndices;

    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = cos(angle);
        float y = sin(angle);
        dartboardVertices.push_back(x * 0.5f);
        dartboardVertices.push_back(y * 0.5f);
        dartboardVertices.push_back(thickness / 2.0f);
        dartboardVertices.push_back((x + 1.0f) / 2.0f);
        dartboardVertices.push_back((y + 1.0f) / 2.0f);
    }
    for (int i = 1; i < segments; ++i) {
        dartboardIndices.push_back(0);
        dartboardIndices.push_back(i);
        dartboardIndices.push_back(i + 1);
    }

    unsigned int dartVAO, dartVBO, dartEBO;
    glGenVertexArrays(1, &dartVAO);
    glGenBuffers(1, &dartVBO);
    glGenBuffers(1, &dartEBO);
    glBindVertexArray(dartVAO);
    glBindBuffer(GL_ARRAY_BUFFER, dartVBO);
    glBufferData(GL_ARRAY_BUFFER, dartboardVertices.size() * sizeof(float), dartboardVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dartEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, dartboardIndices.size() * sizeof(unsigned int), dartboardIndices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    Dart dart;
    dart.Init();
    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        processInput(window);
        camera.Inputs(window);

        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        dart.Update(deltaTime);

        glClearColor(0.85f, 0.85f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Activate();
        camera.Matrix(45.0f, 0.1f, 100.0f, shader, "cameraMatrix");

        // Wall
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        glm::mat4 wallModel = glm::mat4(1.0f);
        int modelLoc = glGetUniformLocation(shader.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(wallModel));
        glBindVertexArray(wallVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Dartboard
        glBindTexture(GL_TEXTURE_2D, dartboardTexture);
        glm::mat4 dartboardModel = glm::mat4(1.0f);
        dartboardModel = glm::translate(dartboardModel, glm::vec3(0.0f, 0.0f, 0.026f));
        dartboardModel = glm::scale(dartboardModel, glm::vec3(scale));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(dartboardModel));
        glBindVertexArray(dartVAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(dartboardIndices.size()), GL_UNSIGNED_INT, 0);

        // Dart
        dart.Draw(shader.ID);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &wallVAO);
    glDeleteBuffers(1, &wallVBO);
    glDeleteBuffers(1, &wallEBO);
    glDeleteVertexArrays(1, &dartVAO);
    glDeleteBuffers(1, &dartVBO);
    glDeleteBuffers(1, &dartEBO);
    glDeleteTextures(1, &wallTexture);
    glDeleteTextures(1, &dartboardTexture);
    shader.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
