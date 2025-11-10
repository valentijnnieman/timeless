#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "timeless/components/component.hpp"

class Texture : public Component
{
public:
    unsigned int texture;
    int width, height;
    std::string filename;

    Texture(const char *filename, int width, int height);
    Texture(const std::vector<uint8_t>& buffer, int width, int height);
    
    void render();
};
