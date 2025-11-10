#pragma once
#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "timeless/components/component.hpp"
#include <map>
#include <string>
#include <vector>
#include <cstdint>

struct Glyph
{
    unsigned int textureId;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

class Font : public Component
{
public:
    unsigned int VAO, VBO;
    std::map<char, Glyph> glyphs;

    Font(int fontSize, const std::string& filepath = "Assets/fonts/8bit.ttf");
    Font(int fontSize, const std::vector<uint8_t>& buffer);
};
