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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model.h"

//Stale potrzebne do 
const double M_PI = 3.14159265358979323846;
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
//Zainicjalizowanie kamery 
Camera camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 0.3f, 5.5f));
float scale = 1.0f;

//Zmiana skali 
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        scale -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        scale += 0.01f;

    if (scale < 0.1f) scale = 0.1f;
    if (scale > 5.0f) scale = 5.0f;
}

//Funkcja do ladowania tekstur
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
        std::cout << "Poprawne loadowanie: " << path << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

int main() {
    //Standardowy setup okna glfw 
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
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    //Dodanie shaderow zwyklego i swiatla 
    Shader shader("default.vert", "default.frag");
    Shader lightShader("Light.vert", "Light.frag");

    //Dodanie modelu lotki 
    Model dart("dart.obj");

    // Framebuffer dla post-processingu
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Tekstura koloru
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // Renderbuffer dla g³êbokoœci
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Prostok¹t pe³noekranowy
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Shader post-processingu (wyostrzanie)
    Shader postShader("postprocessing.vert", "sharpening.frag");
    postShader.Activate();
    postShader.setInt("screenTexture", 0);

    shader.Activate();
    shader.setInt("texture1", 0);

    // --- Generowanie tarczy 3D ---
    const unsigned int segments = 64;
    const float radius = 0.5f;
    const float thickness = 0.05f;
    std::vector<float> dartboardVertices;
    std::vector<unsigned int> dartboardIndices;

    for (unsigned int i = 0; i <= segments; ++i) {
        float angle = 2.0f * static_cast<float>(M_PI) * i / segments;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;

        // Przednia strona (z tekstur¹)
        dartboardVertices.insert(dartboardVertices.end(), {
            x, y, thickness / 2.0f,               // pozycja
            (x / radius + 1.0f) / 2.0f,           // Tekstura U
            (y / radius + 1.0f) / 2.0f,           // Tekstura V
            0.0f, 0.0f, 1.0f                     // normalna
            });

        // Tylna strona (bez tekstury)
        dartboardVertices.insert(dartboardVertices.end(), {
            x, y, -thickness / 2.0f,              // pozycja
            0.0f, 0.0f,                         // tekstura (nieu¿ywane)
            0.0f, 0.0f, -1.0f                   // normalna
            });
    }

    unsigned int frontCenterIndex = static_cast<unsigned int>(dartboardVertices.size() / 8);
    dartboardVertices.insert(dartboardVertices.end(), {
        0.0f, 0.0f, thickness / 2.0f, // pozycja
        0.5f, 0.5f,                  // tekstura
        0.0f, 0.0f, 1.0f             // normalna
        });
    unsigned int backCenterIndex = static_cast<unsigned int>(dartboardVertices.size() / 8);
    dartboardVertices.insert(dartboardVertices.end(), {
        0.0f, 0.0f, -thickness / 2.0f, // pozycja
        0.0f, 0.0f,                  // tekstura (nieu¿ywane)
        0.0f, 0.0f, -1.0f            // normalna
        });

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

    unsigned int dartVAO, dartVBO, dartEBO;
    glGenVertexArrays(1, &dartVAO);
    glGenBuffers(1, &dartVBO);
    glGenBuffers(1, &dartEBO);
    glBindVertexArray(dartVAO);
    glBindBuffer(GL_ARRAY_BUFFER, dartVBO);
    glBufferData(GL_ARRAY_BUFFER, dartboardVertices.size() * sizeof(float), dartboardVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dartEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, dartboardIndices.size() * sizeof(unsigned int), dartboardIndices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    unsigned int indexCount = static_cast<unsigned int>(dartboardIndices.size());

    unsigned int texDartboard = loadTexture("dartboard.png");
    unsigned int texWall = loadTexture("wall.png");
    unsigned int texWallSide = loadTexture("sciana_drewno.png");
    unsigned int texCarpet = loadTexture("podloga_dywan.png");
    unsigned int texKomoda = loadTexture("komoda.jpg");
    unsigned int texKomodaFront = loadTexture("przod_komody.png");

    // Œciana
    float wallDepth = 0.1f;
    float wallSize = 7.0f;
    float wallVerts[] = {
        // Przednia strona
        -wallSize, -wallSize, -wallDepth, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         wallSize, -wallSize, -wallDepth, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         wallSize,  wallSize, -wallDepth, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -wallSize,  wallSize, -wallDepth, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        // Tylna strona
        -wallSize, -wallSize, -wallDepth, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
         wallSize, -wallSize, -wallDepth, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
         wallSize,  wallSize, -wallDepth, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        -wallSize,  wallSize, -wallDepth, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
    };
    unsigned int wallIdx[] = {
        0u, 1u, 2u, 2u, 3u, 0u,
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // --- Pokój (prostopad³oœcian) ---
    float roomW = 7.0f, roomH = 3.0f, roomD = 15.0f;
    float roomVerts[] = {
        // Pod³oga
        -roomW, -roomH, -roomD,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         roomW, -roomH, -roomD,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         roomW, -roomH,  roomD,  1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -roomW, -roomH,  roomD,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        // Sufit
        -roomW,  roomH, -roomD,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
         roomW,  roomH, -roomD,  1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
         roomW,  roomH,  roomD,  1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        -roomW,  roomH,  roomD,  0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        // Œciana tylna
        -roomW, -roomH, -roomD,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         roomW, -roomH, -roomD,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         roomW,  roomH, -roomD,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -roomW,  roomH, -roomD,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        // Œciana przednia
        -roomW, -roomH,  roomD,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
         roomW, -roomH,  roomD,  1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
         roomW,  roomH,  roomD,  1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        -roomW,  roomH,  roomD,  0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        // Œciana lewa
        -roomW, -roomH, -roomD,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -roomW, -roomH,  roomD,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -roomW,  roomH,  roomD,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -roomW,  roomH, -roomD,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        // Œciana prawa
         roomW, -roomH, -roomD,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
         roomW, -roomH,  roomD,  1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
         roomW,  roomH,  roomD,  1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
         roomW,  roomH, -roomD,  0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    };
    unsigned int roomIdx[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9,10,10,11, 8,
       12,13,14,14,15,12,
       16,17,18,18,19,16,
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
    float komodaW = 4.0f, komodaH = 2.0f, komodaD = 1.0f;
    float komodaVerts[] = {
        // Przednia i tylna œciana
        -komodaW / 2, -roomH, -komodaD / 2, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
         komodaW / 2, -roomH, -komodaD / 2, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
         komodaW / 2, -roomH + komodaH, -komodaD / 2, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        -komodaW / 2, -roomH + komodaH, -komodaD / 2, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        // Przód
        -komodaW / 2, -roomH,  komodaD / 2, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         komodaW / 2, -roomH,  komodaD / 2, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         komodaW / 2, -roomH + komodaH,  komodaD / 2, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -komodaW / 2, -roomH + komodaH,  komodaD / 2, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };
    unsigned int komodaIdx[] = {
        // Tylna œciana
        0u, 1u, 2u, 2u, 3u, 0u,
        // Przód
        4u, 5u, 6u, 6u, 7u, 4u,
        // Boki
        0u, 1u, 5u, 5u, 4u, 0u,
        2u, 3u, 7u, 7u, 6u, 2u,
        0u, 3u, 7u, 7u, 4u, 0u,
        1u, 2u, 6u, 6u, 5u, 1u
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Œwiat³o (szeœcian)
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

    // Pozycje Ÿróde³ œwiat³a
    glm::vec3 lights[5] = {
        glm::vec3(0.0f, 1.6f, 0.0f),      // Œrodek nad tarcz¹
        glm::vec3(6.9f, 2.0f, 6.0f),       // Prawa œciana
        glm::vec3(-6.9f, 2.0f, 6.0f),      // Lewa œciana
        glm::vec3(6.9f, 2.0f, 3.0f),       // Prawa œciana bli¿ej
        glm::vec3(-6.9f, 2.0f, 3.0f)       // Lewa œciana bli¿ej
    };

    glm::vec3 colors[5] = {
        glm::vec3(0.0f, 0.0f, 1.0f),       // Niebieski kolor dla lampy nad tarcz¹
        glm::vec3(1.0f, 1.0f, 1.0f),       // Bia³e œwiat³o dla pozosta³ych lamp
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f)
    };

    //umozliwia rotacje wokol wlasnej osi lotki
    float dartRotation = 0.0f;
    float dartFlight = 5.000001f; //leci sobie do tarczy 
    bool dartShouldFly = false; //czy lotka ma leciec 


    while (!glfwWindowShouldClose(window)) {
        //Zaczecie typowe
        glfwPollEvents();
        processInput(window);


        camera.Inputs(window);  //Zostawic ??

        // 1. Renderuj scenê do framebuffera
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.85f, 0.85f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Rysowanie lamp
        lightShader.Activate();
        for (int i = 0; i < 5; i++) {
            glm::mat4 lampModel = glm::mat4(1.0f);
            lampModel = glm::translate(lampModel, lights[i]);
            if (i == 0)
                lampModel = glm::scale(lampModel, glm::vec3(1.0f, 0.2f, 0.2f));
            else
                lampModel = glm::scale(lampModel, glm::vec3(0.2f, 1.0f, 0.2f));
            glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lampModel));
            camera.Matrix(45.0f, 0.1f, 100.0f, lightShader, "cameraMatrix");
            glUniform3fv(glGetUniformLocation(lightShader.ID, "lightColor"), 1, glm::value_ptr(colors[i]));
            glBindVertexArray(lightVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
        shader.setBool("useTexture", true);
        // Rysowanie sceny (pokój, œciany, tarcza, komoda)  
        shader.Activate();
        camera.Matrix(45.0f, 0.1f, 100.0f, shader, "cameraMatrix");
        glUniform1i(glGetUniformLocation(shader.ID, "numLights"), 5);
        glUniform3fv(glGetUniformLocation(shader.ID, "lightPositions"), 5, &lights[0].x);
        glUniform3fv(glGetUniformLocation(shader.ID, "lightColors"), 5, &colors[0].x);
        glUniform3fv(glGetUniformLocation(shader.ID, "viewPos"), 1, glm::value_ptr(camera.Position));

        // Pokój
        glm::mat4 roomM = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(roomM));
        glBindVertexArray(roomVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texCarpet);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texWallSide);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(unsigned int)));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texWall);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(12 * sizeof(unsigned int)));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texWall);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(18 * sizeof(unsigned int)));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texWallSide);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(24 * sizeof(unsigned int)));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texWallSide);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(30 * sizeof(unsigned int)));

        // Œciana
        glm::mat4 wallM = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(wallM));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texWall);
        glBindVertexArray(wallVAO);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

        // Tarcza
        glm::mat4 dartboardModel = glm::mat4(1.0f);
        dartboardModel = glm::translate(dartboardModel, glm::vec3(0.0f, 0.0f, -wallDepth + thickness / 2));
        dartboardModel = glm::scale(dartboardModel, glm::vec3(scale));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(dartboardModel));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texDartboard);
        glBindVertexArray(dartVAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

        // Komoda
        glm::mat4 komodaModel = glm::mat4(1.0f);
        komodaModel = glm::translate(komodaModel, glm::vec3(6.5f, 0, 9.0f));
        komodaModel = glm::rotate(komodaModel, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(komodaModel));
        glBindVertexArray(komodaVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texKomodaFront);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texKomoda);
        glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, (void*)(6 * sizeof(unsigned int)));

        

        //Animacja podazania kamery za lotka
        glm::vec3 dartPosition = glm::vec3(0.0f, 0.0f, dartFlight);
        glm::vec3 cameraOffset = glm::vec3(2.0f, 2.0f, 4.0f); // dostosuj wed³ug uznania
        camera.FollowTarget(dartPosition, cameraOffset);


        //Wylaczenie tesktury na czas rysowania lotki 
        shader.setBool("useTexture", false);
        shader.setVec3("overrideColor", glm::vec3(0.0f, 0.0f, 1.0f)); // czerwony


        //Renderowanie lotki 
        glm::mat4 dartTransform = glm::mat4(1.0f);
        dartTransform = glm::translate(dartTransform, glm::vec3(0.0f, 0.0f, dartFlight));
        if (dartFlight > 0.0f && dartFlight < 5.0f) {
            dartTransform = glm::rotate(dartTransform, glm::radians(dartRotation), glm::vec3(0.0f, 0.0f, 1.0f));
        }
        dartTransform = glm::scale(dartTransform, glm::vec3(0.05f)); // dostosuj skalê

        int modelLoc1 = glGetUniformLocation(shader.ID, "model");
        glUniformMatrix4fv(modelLoc1, 1, GL_FALSE, glm::value_ptr(dartTransform));

        dart.Draw(shader);

        shader.Activate();
        camera.Matrix(45.0f, 0.1f, 100.0f, shader, "cameraMatrix");
        shader.setBool("useTexture", true);
        shader.setInt("texture1", 0);
        glActiveTexture(GL_TEXTURE0);

        glUniform3fv(glGetUniformLocation(shader.ID, "viewPos"), 1, glm::value_ptr(camera.Position));
        glUniform1i(glGetUniformLocation(shader.ID, "numLights"), 5);
        glUniform3fv(glGetUniformLocation(shader.ID, "lightPositions"), 5, &lights[0].x);
        glUniform3fv(glGetUniformLocation(shader.ID, "lightColors"), 5, &colors[0].x);
 

        //Przed zamiana okna i ponownym rysowaniu zmienia o 1 stopien kat obortu lotki 
        if (dartFlight > 4.8f && dartFlight < 5.0) {
            dartRotation += 0.1f;
        }
        else if (dartFlight > 4.5f && dartFlight < 5.0) {
            dartRotation += 0.2f;
        }
        else if (dartFlight > 4.0f && dartFlight < 5.0) {
            dartRotation += 0.3f;
        }
        else if (dartFlight > 3.5f && dartFlight < 5.0) {
            dartRotation += 0.4f;
        }
        else {
            dartRotation += 0.5f;
        }
        if (dartRotation >= 360.0f) dartRotation -= 360.0f; //jesli dojdzie do 360 wraca do zera 
        
        //Sprawdzenie czy dalej ma leciec 
        if (dartShouldFly && dartFlight > -0.1f) {
            dartFlight -= 0.005f;
        }
        if (dartFlight <= -0.1f) {
            dartFlight = -0.1f;
            dartShouldFly = false; // Zatrzymaj ruch po dotarciu do tarczy
        }

        //Dodanie resetowania lotu lotki 
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            dartShouldFly = false;
            dartFlight = 5.000001f;
            dartRotation = 0.0f;
        }

        // Uruchom lotkê przy naciœniêciu spacji
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            dartShouldFly = true;
        
        // 2. Post-processing: rysuj quad na ekranie
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        postShader.Activate();
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
    }

    // --- Sprz¹tanie zasobów ---
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
    glDeleteTextures(1, &texWallSide);
    glDeleteTextures(1, &texCarpet);
    glDeleteTextures(1, &texKomoda);
    glDeleteTextures(1, &texKomodaFront);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteRenderbuffers(1, &rbo);
    shader.Delete();
    lightShader.Delete();
    postShader.Delete();
    glfwTerminate();
    return 0;
}
