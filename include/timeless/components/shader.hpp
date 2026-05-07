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
#include <unordered_map>
#include <cstdint>
#include "timeless/components/component.hpp"

class Shader : public Component
{
private:
    std::unordered_map<std::string, GLint> uniform_cache;
    std::unordered_map<std::string, GLint> attrib_cache;

public:
    unsigned int ID;

    Shader(const char *vertexPath, const char *fragmentPath);
    Shader(const std::vector<uint8_t>& vertexBuffer, const std::vector<uint8_t>& fragmentBuffer);
    void use();

    GLint get_uniform(const std::string& name) {
        auto it = uniform_cache.find(name);
        if (it != uniform_cache.end()) return it->second;
        GLint loc = glGetUniformLocation(ID, name.c_str());
        uniform_cache[name] = loc;
        return loc;
    }

    GLint get_attrib(const std::string& name) {
        auto it = attrib_cache.find(name);
        if (it != attrib_cache.end()) return it->second;
        GLint loc = glGetAttribLocation(ID, name.c_str());
        attrib_cache[name] = loc;
        return loc;
    }
};

#endif
