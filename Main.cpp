#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderClass.h"
#include "Camera.h"
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
const double M_PI = 3.14159265358979323846;

Camera camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 0.0f, 12.0f));
float scale = 1.0f;

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        scale -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        scale += 0.01f;
    scale = std::max(0.1f, std::min(scale, 5.0f));
    camera.Inputs(window);
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Dartboard Scene with Light", NULL, NULL);
    if (!window) { std::cout << "GLFW init fail\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE); // Wy³¹czamy culling, aby renderowaæ obie strony

    Shader shader("default.vert", "default.frag");
    Shader lightShader("Light.vert", "Light.frag");
    shader.Activate();
    shader.setInt("texture1", 0);

    // --- Generowanie tarczy 3D
    const unsigned int segments = 64;
    const float radius = 0.5f;
    const float thickness = 0.05f;

    std::vector<float> dartboardVertices;
    std::vector<unsigned int> dartboardIndices;

    // Wierzcho³ki i indeksy
    for (unsigned int i = 0; i <= segments; ++i) {
        float angle = 2.0f * static_cast<float>(M_PI) * i / segments;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;

        // Przednia strona (z tekstur¹)
        dartboardVertices.insert(dartboardVertices.end(), {
            x, y, thickness / 2.0f,               // Pozycja
            (x / radius + 1.0f) / 2.0f,           // Tekstura U
            (y / radius + 1.0f) / 2.0f,           // Tekstura V
            0.0f, 0.0f, 1.0f                      // Normalna
            });

        // Tylna strona (bez tekstury)
        dartboardVertices.insert(dartboardVertices.end(), {
            x, y, -thickness / 2.0f,              // Pozycja
            0.0f, 0.0f,                           // Tekstura (nieu¿ywane)
            0.0f, 0.0f, -1.0f                     // Normalna
            });
    }

    unsigned int frontCenterIndex = static_cast<unsigned int>(dartboardVertices.size() / 8);
    dartboardVertices.insert(dartboardVertices.end(), {
        0.0f, 0.0f, thickness / 2.0f,   // Pozycja
        0.5f, 0.5f,                     // Tekstura
        0.0f, 0.0f, 1.0f                // Normalna
        });
    unsigned int backCenterIndex = static_cast<unsigned int>(dartboardVertices.size() / 8);
    dartboardVertices.insert(dartboardVertices.end(), {
        0.0f, 0.0f, -thickness / 2.0f,  // Pozycja
        0.0f, 0.0f,                     // Tekstura (nieu¿ywane)
        0.0f, 0.0f, -1.0f               // Normalna
        });

    // Indeksy
    for (unsigned int i = 0; i < segments; ++i) {
        unsigned int idx = i * 2;
        unsigned int nextIdx = (idx + 2) % ((segments + 1) * 2);
        unsigned int nextIdx1 = (idx + 3) % ((segments + 1) * 2);
        // Przednia strona
        dartboardIndices.insert(dartboardIndices.end(), {
            frontCenterIndex, idx, nextIdx
            });
        // Tylna strona
        dartboardIndices.insert(dartboardIndices.end(), {
            backCenterIndex, nextIdx1, idx + 1
            });
        // Boki
        dartboardIndices.insert(dartboardIndices.end(), {
            idx, nextIdx, idx + 1
            });
        dartboardIndices.insert(dartboardIndices.end(), {
            idx + 1, nextIdx, nextIdx1
            });
    }

    // VAO dla tarczy
    unsigned int dartVAO, dartVBO, dartEBO;
    glGenVertexArrays(1, &dartVAO);
    glGenBuffers(1, &dartVBO);
    glGenBuffers(1, &dartEBO);
    glBindVertexArray(dartVAO);
    glBindBuffer(GL_ARRAY_BUFFER, dartVBO);
    glBufferData(GL_ARRAY_BUFFER, dartboardVertices.size() * sizeof(float), dartboardVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dartEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, dartboardIndices.size() * sizeof(unsigned int), dartboardIndices.data(), GL_STATIC_DRAW);
    // Pozycje
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Tekstury
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Normalne
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int indexCount = static_cast<unsigned int>(dartboardIndices.size());

    unsigned int texDartboard = loadTexture("dartboard.png");
    unsigned int texWall = loadTexture("wall.png"); // œciana z tarcz¹ i naprzeciwko
    unsigned int texWallSide = loadTexture("sciana_drewno.png"); // boczne œciany i sufit
    unsigned int texCarpet = loadTexture("podloga_dywan.png");
    unsigned int texKomoda = loadTexture("komoda.jpg");

    // Œciana
    float wallDepth = 0.1f;
    float wallSize = 7.0f; // lub tyle, ile wynosi roomW
    float wallVerts[] = {
        // Pozycje                   // Tekstury   // Normalne
        // Przednia strona
        -wallSize, -wallSize, -wallDepth, 0.0f, 0.0f,  0.0f,  0.0f, 1.0f,
         wallSize, -wallSize, -wallDepth, 1.0f, 0.0f,  0.0f,  0.0f, 1.0f,
         wallSize,  wallSize, -wallDepth, 1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
        -wallSize,  wallSize, -wallDepth, 0.0f, 1.0f,  0.0f,  0.0f, 1.0f,
        // Tylna strona
        -wallSize, -wallSize, -wallDepth, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         wallSize, -wallSize, -wallDepth, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         wallSize,  wallSize, -wallDepth, 1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -wallSize,  wallSize, -wallDepth, 0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
    };
    unsigned int wallIdx[] = {
        // Przednia strona
        0u, 1u, 2u, 2u, 3u, 0u,
        // Tylna strona
        4u, 7u, 6u, 6u, 5u, 4u
    };
    unsigned int wallVAO, wallVBO, wallEBO;
    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);
    glGenBuffers(1, &wallEBO);
    glBindVertexArray(wallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wallVerts), wallVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(wallIdx), wallIdx, GL_STATIC_DRAW);
    // Pozycje
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Tekstury
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Normalne
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // --- Pokój (prostopad³oœcian) ---
    float roomW = 7.0f, roomH = 3.0f, roomD = 15.0f;
    float roomVerts[] = {
        // Pod³oga (y = -roomH)
        -roomW, -roomH, -roomD,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         roomW, -roomH, -roomD,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         roomW, -roomH,  roomD,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
        -roomW, -roomH,  roomD,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
        // Sufit (y = +roomH)
        -roomW,  roomH, -roomD,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
         roomW,  roomH, -roomD,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
         roomW,  roomH,  roomD,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f,
        -roomW,  roomH,  roomD,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,
        // Œciana tylna (z = -roomD)
        -roomW, -roomH, -roomD,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         roomW, -roomH, -roomD,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         roomW,  roomH, -roomD,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -roomW,  roomH, -roomD,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        // Œciana przednia (z = +roomD)
        -roomW, -roomH,  roomD,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
         roomW, -roomH,  roomD,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
         roomW,  roomH,  roomD,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        -roomW,  roomH,  roomD,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        // Œciana lewa (x = -roomW)
        -roomW, -roomH, -roomD,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        -roomW, -roomH,  roomD,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        -roomW,  roomH,  roomD,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
        -roomW,  roomH, -roomD,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
        // Œciana prawa (x = +roomW)
         roomW, -roomH, -roomD,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
         roomW, -roomH,  roomD,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
         roomW,  roomH,  roomD,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
         roomW,  roomH, -roomD,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
    };
    unsigned int roomIdx[] = {
        // Pod³oga
        0, 1, 2, 2, 3, 0,
        // Sufit
        4, 5, 6, 6, 7, 4,
        // Tylna
        8, 9,10,10,11, 8,
        // Przednia
       12,13,14,14,15,12,
        // Lewa
       16,17,18,18,19,16,
        // Prawa
       20,21,22,22,23,20
    };
    unsigned int roomVAO, roomVBO, roomEBO;
    glGenVertexArrays(1, &roomVAO);
    glGenBuffers(1, &roomVBO);
    glGenBuffers(1, &roomEBO);
    glBindVertexArray(roomVAO);
    glBindBuffer(GL_ARRAY_BUFFER, roomVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roomVerts), roomVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, roomEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(roomIdx), roomIdx, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Komoda
    float komodaW = 4.0f, komodaH = 2.0f, komodaD = 2.0f;
    float komodaVerts[] = {
        // pozycje           // tekstury
        -komodaW/2, -roomH, -komodaD/2, 0.0f, 0.0f,
         komodaW/2, -roomH, -komodaD/2, 1.0f, 0.0f,
         komodaW/2, -roomH+komodaH, -komodaD/2, 1.0f, 1.0f,
        -komodaW/2, -roomH+komodaH, -komodaD/2, 0.0f, 1.0f,
        -komodaW/2, -roomH, komodaD/2, 0.0f, 0.0f,
         komodaW/2, -roomH, komodaD/2, 1.0f, 0.0f,
         komodaW/2, -roomH+komodaH, komodaD/2, 1.0f, 1.0f,
        -komodaW/2, -roomH+komodaH, komodaD/2, 0.0f, 1.0f,
    };
    unsigned int komodaIdx[] = {
        0,1,2, 2,3,0, // ty³
        4,5,6, 6,7,4, // przód
        0,1,5, 5,4,0, // dó³
        2,3,7, 7,6,2, // góra
        0,3,7, 7,4,0, // lewo
        1,2,6, 6,5,1  // prawo
    };
    unsigned int komodaVAO, komodaVBO, komodaEBO;
    glGenVertexArrays(1, &komodaVAO);
    glGenBuffers(1, &komodaVBO);
    glGenBuffers(1, &komodaEBO);
    glBindVertexArray(komodaVAO);
    glBindBuffer(GL_ARRAY_BUFFER, komodaVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(komodaVerts), komodaVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, komodaEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(komodaIdx), komodaIdx, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Œwiat³o
    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };
    unsigned int cubeIndices[] = {
        0u, 1u, 2u, 2u, 3u, 0u, // Ty³
        4u, 5u, 6u, 6u, 7u, 4u, // Przód
        0u, 1u, 5u, 5u, 4u, 0u, // Dó³
        2u, 3u, 7u, 7u, 6u, 2u, // Góra
        0u, 3u, 7u, 7u, 4u, 0u, // Lewo
        1u, 2u, 6u, 6u, 5u, 1u  // Prawo
    };
    unsigned int lightVAO, lightVBO, lightEBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glGenBuffers(1, &lightEBO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glm::vec3 lightPos = glm::vec3(1.2f, 1.0f, 2.0f);
    glm::vec3 lightColor = glm::vec3(1.0f);

    // Dodaj przed pêtl¹ g³ówn¹ (np. obok lightPos)
    glm::vec3 endrodLight1 = glm::vec3(7.0f, 2.0f, 6.0f);
    glm::vec3 endrodLight2 = glm::vec3(-7.0f, 2.0f, 6.0f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processInput(window);

        glClearColor(0.85f, 0.85f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Renderowanie Ÿród³a œwiat³a
        lightShader.Activate();
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, lightPos);
        lightModel = glm::scale(lightModel, glm::vec3(0.2f));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
        camera.Matrix(45.0f, 0.1f, 100.0f, lightShader, "cameraMatrix");
        glBindVertexArray(lightVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Renderowanie endrodów jako œwiat³a (np. jako bia³e pod³u¿ne prostopad³oœciany)
        lightShader.Activate();
        glm::mat4 endrodModel1 = glm::mat4(1.0f);
        endrodModel1 = glm::translate(endrodModel1, endrodLight1);
        endrodModel1 = glm::scale(endrodModel1, glm::vec3(0.2f, 1.0f, 0.2f)); // pod³u¿ny prêt
        glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(endrodModel1));
        camera.Matrix(45.0f, 0.1f, 100.0f, lightShader, "cameraMatrix");
        glBindVertexArray(lightVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glm::mat4 endrodModel2 = glm::mat4(1.0f);
        endrodModel2 = glm::translate(endrodModel2, endrodLight2);
        endrodModel2 = glm::scale(endrodModel2, glm::vec3(0.2f, 1.0f, 0.2f));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(endrodModel2));
        camera.Matrix(45.0f, 0.1f, 100.0f, lightShader, "cameraMatrix");
        glBindVertexArray(lightVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Renderowanie pokoju (œcian, pod³ogi, sufitu)
        shader.Activate();
        camera.Matrix(45.0f, 0.1f, 100.0f, shader, "cameraMatrix");
        glUniform3fv(glGetUniformLocation(shader.ID, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shader.ID, "viewPos"), 1, glm::value_ptr(camera.Position));
        glUniform3fv(glGetUniformLocation(shader.ID, "lightColor"), 1, glm::value_ptr(lightColor));
        glm::mat4 roomM = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(roomM));
        glBindVertexArray(roomVAO);

        // Pod³oga (indeksy 0-5) - dywan
        glBindTexture(GL_TEXTURE_2D, texCarpet);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));

        // Sufit (indeksy 6-11) - drewno
        glBindTexture(GL_TEXTURE_2D, texWallSide);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(unsigned int)));

        // Œciana tylna (indeksy 12-17) - wall.jpg
        glBindTexture(GL_TEXTURE_2D, texWall);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(12 * sizeof(unsigned int)));

        // Œciana przednia (indeksy 18-23) - wall.jpg
        glBindTexture(GL_TEXTURE_2D, texWall);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(18 * sizeof(unsigned int)));

        // Œciana lewa (indeksy 24-29) - drewno
        glBindTexture(GL_TEXTURE_2D, texWallSide);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(24 * sizeof(unsigned int)));

        // Œciana prawa (indeksy 30-35) - drewno
        glBindTexture(GL_TEXTURE_2D, texWallSide);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(30 * sizeof(unsigned int)));

        // Renderowanie œciany
        glm::mat4 wallM = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(wallM));
        glBindTexture(GL_TEXTURE_2D, texWall);
        glBindVertexArray(wallVAO);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

        // Renderowanie tarczy
        glm::mat4 dartboardModel = glm::mat4(1.0f);
        dartboardModel = glm::translate(dartboardModel, glm::vec3(0.0f, 0.0f, -wallDepth + thickness / 2));
        dartboardModel = glm::scale(dartboardModel, glm::vec3(scale));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(dartboardModel));
        glBindTexture(GL_TEXTURE_2D, texDartboard);
        glBindVertexArray(dartVAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

        // Renderowanie komody
        shader.Activate();
        glm::mat4 komodaModel = glm::mat4(1.0f);
        // Ustaw komodê na (12.0f, 0.0f,6.0f)
        komodaModel = glm::translate(
            komodaModel,
            glm::vec3(6.0f, 0.0f, 0.0f)
        );
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(komodaModel));
        glBindTexture(GL_TEXTURE_2D, texKomoda);
        glBindVertexArray(komodaVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &dartVAO);
    glDeleteBuffers(1, &dartVBO);
    glDeleteBuffers(1, &dartEBO);
    glDeleteVertexArrays(1, &wallVAO);
    glDeleteBuffers(1, &wallVBO);
    glDeleteBuffers(1, &wallEBO);
    glDeleteVertexArrays(1, &roomVAO);
    glDeleteBuffers(1, &roomVBO);
    glDeleteBuffers(1, &roomEBO);
    glDeleteVertexArrays(1, &komodaVAO);
    glDeleteBuffers(1, &komodaVBO);
    glDeleteBuffers(1, &komodaEBO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &lightVBO);
    glDeleteBuffers(1, &lightEBO);
    glDeleteTextures(1, &texWall);
    glDeleteTextures(1, &texDartboard);
    shader.Delete();
    lightShader.Delete();
    glfwTerminate();
    return 0;
}
