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
#include <memory>
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

    // Returns a shared, path-cached Texture, loading the file only the first
    // time it is requested. Many trials and category labels reference the same
    // image, so this avoids loading (and GL-uploading) identical textures more
    // than once. Cached textures live until the program exits.
    static std::shared_ptr<Texture> get_cached(const std::string &filename,
                                               int width, int height);

    void render();
};
