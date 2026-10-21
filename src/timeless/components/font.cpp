#include "timeless/components/font.hpp"
#include <algorithm>
#include <iostream>
#include <filesystem>

void Font::init_buffers()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Renders ASCII 0-127 and packs them into one texture, row by row (shelf
// packing), with a gap between glyphs so linear filtering never pulls in a
// neighbour's pixels.
void Font::load_glyphs(FT_Face face, int fontSize)
{
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    this->line_height = (face->size->metrics.height >> 6);

    struct Bitmap
    {
        unsigned char c;
        int width, rows;
        std::vector<unsigned char> pixels;
    };
    std::vector<Bitmap> bitmaps;
    std::map<char, Glyph> gen_glyphs;
    long area = 0;
    int widest = 0;
    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYPE: Failed to load glyph for character " << c << std::endl;
            continue;
        }
        FT_Bitmap &bm = face->glyph->bitmap;
        Glyph glyph = {
            glm::vec4(0.0f),
            glm::ivec2(bm.width, bm.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)};
        gen_glyphs.insert(std::pair<char, Glyph>(c, glyph));
        if (bm.width == 0 || bm.rows == 0)
            continue;
        Bitmap b{c, (int)bm.width, (int)bm.rows, {}};
        b.pixels.resize(b.width * b.rows);
        for (int row = 0; row < b.rows; row++)
            std::copy_n(bm.buffer + row * bm.pitch, b.width,
                        b.pixels.begin() + row * b.width);
        area += long(b.width + PADDING) * (b.rows + PADDING);
        widest = std::max(widest, b.width + PADDING);
        bitmaps.push_back(std::move(b));
    }

    // A power-of-two width that makes the atlas roughly square.
    int width = 64;
    while (long(width) * width < area * 2 || width < widest + PADDING)
        width *= 2;
    // Tallest first, so each row wastes little height.
    std::sort(bitmaps.begin(), bitmaps.end(),
              [](const Bitmap &a, const Bitmap &b) { return a.rows > b.rows; });
    std::vector<glm::ivec2> places(bitmaps.size());
    int x = PADDING, y = PADDING, row_height = 0;
    for (size_t i = 0; i < bitmaps.size(); i++)
    {
        if (x + bitmaps[i].width + PADDING > width)
        {
            x = PADDING;
            y += row_height + PADDING;
            row_height = 0;
        }
        places[i] = glm::ivec2(x, y);
        x += bitmaps[i].width + PADDING;
        row_height = std::max(row_height, bitmaps[i].rows);
    }
    int height = y + row_height + PADDING;

    std::vector<unsigned char> pixels(size_t(width) * height, 0);
    for (size_t i = 0; i < bitmaps.size(); i++)
    {
        const Bitmap &b = bitmaps[i];
        for (int row = 0; row < b.rows; row++)
            std::copy_n(b.pixels.begin() + row * b.width, b.width,
                        pixels.begin() + size_t(places[i].y + row) * width + places[i].x);
        gen_glyphs[b.c].uv = glm::vec4(
            float(places[i].x) / width, float(places[i].y) / height,
            float(places[i].x + b.width) / width, float(places[i].y + b.rows) / height);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &atlas);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED,
                 GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    this->glyphs = gen_glyphs;
}

Font::Font(int fontSize, const std::string& filepath)
{
    init_buffers();
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    std::string font_name = std::filesystem::path(filepath).string();

    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }
    else
    {
        load_glyphs(face, fontSize);
        FT_Done_Face(face);
    }
    FT_Done_FreeType(ft);
}

Font::Font(int fontSize, const std::vector<uint8_t>& buffer)
{
    init_buffers();
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    FT_Face face;
    if (FT_New_Memory_Face(ft, buffer.data(), buffer.size(), 0, &face)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font from memory" << std::endl;
        FT_Done_FreeType(ft);
        return;
    }

    load_glyphs(face, fontSize);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}
