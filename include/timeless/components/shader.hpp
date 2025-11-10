#pragma once

#ifndef SHADER_H
#define SHADER_H

#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <cstdint>
#include "timeless/components/component.hpp"

class Shader : public Component
{
public:
    unsigned int ID;
    
    Shader(const char *vertexPath, const char *fragmentPath);
    Shader(const std::vector<uint8_t>& vertexBuffer, const std::vector<uint8_t>& fragmentBuffer);
    void use();
};

#endif
