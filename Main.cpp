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
#include <algorithm>


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
    scale = std::max(0.1f, std::min(scale, 5.0f));
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

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Dartboard Scene", NULL, NULL);
    if (!window) { std::cout << "GLFW init fail\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    Shader shader("default.vert", "default.frag");
    shader.Activate();
    shader.setInt("texture1", 0);

    Dart dart;
    float lastTime = glfwGetTime();


    unsigned int texDartboard = loadTexture("dartboard.png");
    unsigned int texWall = loadTexture("wall.png");

    // --- Wall cube (back wall)
    float wallDepth = 0.1f;
    float wallSize = 5.0f;
    float wallVerts[] = {
        -wallSize, -wallSize, -wallDepth,  0.0f, 0.0f,
         wallSize, -wallSize, -wallDepth,  1.0f, 0.0f,
         wallSize,  wallSize, -wallDepth,  1.0f, 1.0f,
        -wallSize,  wallSize, -wallDepth,  0.0f, 1.0f
    };
    unsigned int wallIdx[] = { 0, 1, 2, 2, 3, 0 };
    unsigned int wallVAO, wallVBO, wallEBO;
    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);
    glGenBuffers(1, &wallEBO);
    glBindVertexArray(wallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wallVerts), wallVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(wallIdx), wallIdx, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // --- Dartboard 3D disk
    std::vector<float> verts;
    std::vector<unsigned int> inds;
    const float radius = 0.5f;
    const float depth = 0.05f;
    const int segments = 64;

    // Front
    verts.insert(verts.end(), { 0, 0, depth/2, 0.5f, 0.5f, 0 });
    for (int i = 0; i <= segments; ++i) {
        float a = 2 * M_PI * i / segments;
        float x = cos(a) * radius;
        float y = sin(a) * radius;
        verts.insert(verts.end(), { x, y, depth/2, (x/radius+1)/2, (y/radius+1)/2, 0 });
    }
    for (unsigned int i = 1; i <= segments; i++) {
        inds.insert(inds.end(), { 0, i, i+1 });
    }

    // Back
    unsigned int backStart = verts.size() / 6;
    verts.insert(verts.end(), { 0, 0, -depth/2, 0.5f, 0.5f, 1 });
    for (int i = 0; i <= segments; ++i) {
        float a = 2 * M_PI * i / segments;
        float x = cos(a) * radius;
        float y = sin(a) * radius;
        verts.insert(verts.end(), { x, y, -depth/2, 0, 0, 1 });
    }
    for (unsigned int i = 1; i <= segments; i++) {
        inds.insert(inds.end(), { backStart, backStart + i + 1, backStart + i });
    }

    // Sides
    unsigned int sideStart = verts.size() / 6;
    for (int i = 0; i <= segments; i++) {
        float a = 2 * M_PI * i / segments;
        float x = cos(a) * radius;
        float y = sin(a) * radius;
        verts.insert(verts.end(), { x, y, depth/2, 0, 0, 2 });
        verts.insert(verts.end(), { x, y, -depth/2, 0, 0, 2 });
    }
    for (int i = 0; i < segments; i++) {
        unsigned int top = sideStart + i * 2;
        unsigned int bot = top + 1;
        inds.insert(inds.end(), { top, bot, bot+2, top, bot+2, top+2 });
    }

    unsigned int vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned int), inds.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(2, 1, GL_INT, 6 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processInput(window);
        camera.Inputs(window);

        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        dart.Update(deltaTime);
        dart.Draw(shader.ID);

        glClearColor(0.85f, 0.85f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.Activate();
        camera.Matrix(45.0f, 0.1f, 100.0f, shader, "cameraMatrix");



        // WALL
        glm::mat4 wallM = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(wallM));
        glBindTexture(GL_TEXTURE_2D, texWall);
        glBindVertexArray(wallVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // TARCZA
        glm::mat4 tarczaM = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -wallDepth + depth/2));
        tarczaM = glm::scale(tarczaM, glm::vec3(scale));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(tarczaM));
        glBindTexture(GL_TEXTURE_2D, texDartboard);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, inds.size(), GL_UNSIGNED_INT, 0);
        dart.Draw(shader.ID);
        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &vao); glDeleteBuffers(1, &vbo); glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &wallVAO); glDeleteBuffers(1, &wallVBO); glDeleteBuffers(1, &wallEBO);
    glDeleteTextures(1, &texWall); glDeleteTextures(1, &texDartboard);
    shader.Delete(); glfwTerminate();
    return 0;
}
