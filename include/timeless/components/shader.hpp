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

    // Caller must have called use() first — this only uploads to whichever
    // program is currently bound, same as every other raw glUniform* call.
    void set_vec3(const std::string& name, const glm::vec3& v) {
        glUniform3f(get_uniform(name), v.x, v.y, v.z);
    }
};

#endif
