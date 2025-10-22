#pragma once

#include <string>
#include <iostream>
#include <algorithm>
#include <glm/vec2.hpp>
#include <vector>
#include <map>
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "component.hpp"

#include "stb_image.h"

class Texture : public Component
{
public:
    unsigned int texture;
    int width, height;
    std::string filename;

    Texture(const char *filename, int width, int height)
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
        // set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); 
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR); 
    glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }

    Texture(const std::vector<uint8_t>& buffer, int width, int height)
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
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }

    void render()
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
};
