#include "timeless/components/texture.hpp"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

#include "stb_image.h"

Texture::Texture(const char *filename, int width, int height)
    : width(width), height(height), filename(std::string(filename))
{
    int nrChannels;
    int w = static_cast<int>(width);
    int h = static_cast<int>(height);

    stbi_set_flip_vertically_on_load(false);
    unsigned char *data = stbi_load(filename, &w, &h, &nrChannels, 4);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
// #ifdef __EMSCRIPTEN__
// #else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glGenerateMipmap(GL_TEXTURE_2D);
// #endif

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); 
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR); 
    // glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

Texture::Texture(const std::vector<uint8_t>& buffer, int width, int height)
{
    int nrChannels;
    int w = width;
    int h = height;

    stbi_set_flip_vertically_on_load(false);
    unsigned char *data = stbi_load_from_memory(buffer.data(), buffer.size(), &w, &h, &nrChannels, 4);

    if (!data) {
        std::cerr << "Failed to load texture from memory!" << std::endl;
        texture = 0;
        this->width = 0;
        this->height = 0;
        return;
    }

    this->width = w;
    this->height = h;

    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
// #ifdef __EMSCRIPTEN__
// #else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glGenerateMipmap(GL_TEXTURE_2D);
// #endif

    stbi_image_free(data);

    // GLenum err = glGetError();
    // if (err != GL_NO_ERROR) {
    //     std::cout << "OpenGL error: " << err << std::endl;
    // }
}

void Texture::render()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // GLenum err = glGetError();
    // if (err != GL_NO_ERROR) {
    //     std::cout << "OpenGL error: " << err << " when binding texture " << texture << std::endl;
    // }

    // GLint dwidth = 0;
    // glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &dwidth);
    // if (dwidth == 0) std::cout << "Texture not valid! texture: " << texture << " filename: " << filename << std::endl;
}

