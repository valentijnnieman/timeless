#pragma once

#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif
#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include "timeless/components/component.hpp"

class Texture : public Component
{
public:
    unsigned int texture;
    int width, height;
    std::string filename;

    Texture(const char *filename, int width, int height);
    Texture(const std::vector<uint8_t>& buffer, int width, int height);
    ~Texture()
    {
        glDeleteTextures(1, &texture);
    }
    
    void render();
};
