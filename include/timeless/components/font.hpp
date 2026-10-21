#pragma once
#include "timeless/gl.hpp"
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
    // Where the glyph is in its font's atlas: u0, v0, u1, v1 (v0 the top
    // row). Zero for glyphs with no pixels (space, control characters).
    glm::vec4 uv;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

class Font : public Component
{
public:
    unsigned int VAO, VBO;
    // All glyphs in one texture (GL_R8), so a whole string draws with one
    // texture bound and its quads in one buffer.
    unsigned int atlas = 0;
    std::map<char, Glyph> glyphs;
    int line_height = 0;

    Font(int fontSize, const std::string& filepath = "Assets/fonts/8bit.ttf");
    Font(int fontSize, const std::vector<uint8_t>& buffer);

private:
    // Empty pixels around each glyph in the atlas.
    static constexpr int PADDING = 2;
    void init_buffers();
    void load_glyphs(FT_Face face, int fontSize);
};
