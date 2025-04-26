#include "Texture.h"
#include <iostream>

Texture::Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType)
{
    type = texType;
    int widthImg, heightImg, numColCh;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

    if (!bytes)
    {
        std::cout << "X Nie uda³o siê za³adowaæ tekstury: " << image << std::endl;
        return;
    }
    else
    {
        std::cout << " Za³adowano teksturê: " << image << std::endl;
    }

    glGenTextures(1, &ID);
    glActiveTexture(slot);
    glBindTexture(texType, ID);

    // Ustawienia filtrowania i zawijania
    glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Automatyczne dopasowanie formatu
    GLenum internalFormat = (numColCh == 4) ? GL_RGBA : GL_RGB;
    GLenum inputFormat = (numColCh == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(texType, 0, internalFormat, widthImg, heightImg, 0, inputFormat, pixelType, bytes);
    glGenerateMipmap(texType);

    stbi_image_free(bytes);
    glBindTexture(texType, 0);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
    GLuint texUni = glGetUniformLocation(shader.ID, uniform);
    shader.Activate();
    glUniform1i(texUni, unit);
}

void Texture::Bind()
{
    glBindTexture(type, ID);
}

void Texture::Unbind()
{
    glBindTexture(type, 0);
}

void Texture::Delete()
{
    glDeleteTextures(1, &ID);
}
