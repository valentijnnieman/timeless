#pragma once
#include "../components/font.hpp"
#include "../components/text.hpp"
#include "system.hpp"

class TextRenderingSystem : public System
{
public:
    Entity camera;

    void register_camera(Entity c)
    {
        camera = c;
    }

    float getWidth(Entity entity, ComponentManager &cm)
    {
        auto font = cm.fonts.at(entity);
        auto text = cm.texts.at(entity);

        float width = 0.0f;
        // Get width of string
        std::string::const_iterator c;
        for (c = text->text.begin(); c != text->text.end(); c++)
        {
            Glyph glyph = font->glyphs.at(*c);
            width += glyph.advance >> 6;
        }
        return width;
    }
    float get_width(std::shared_ptr<Font> font, std::string text)
    {
        float width;
        // Get width of string
        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++)
        {
            Glyph glyph = font->glyphs.at(*c);
            width += glyph.advance >> 6;
        }
        return width;
    }

    float getHeight(Entity entity, ComponentManager &cm)
    {
        auto font = cm.fonts.at(entity);
        auto text = cm.texts.at(entity);

        float height = 0.0f;
        // Get width of string
        std::string::const_iterator c;
        for (c = text->text.begin(); c != text->text.end(); c++)
        {
            Glyph glyph = font->glyphs.at(*c);
            height = glyph.size.y;
        }
        return height;
    }

    void set_shader_uniforms(Entity entity, ComponentManager &cm)
    {
        auto shader = cm.shaders.at(entity);
        auto transform = cm.transforms.at(entity);
        auto font = cm.fonts.at(entity);
        auto text = cm.texts.at(entity);
        // glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f);

        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(transform->projection));
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(transform->model));
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE, glm::value_ptr(transform->view));

        //glUniform4f(glGetUniformLocation(shader->ID, "textColor"), text->color.r, text->color.g, text->color.b, text->color.a);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(font->VAO);
    }

    void render(ComponentManager &cm, int x, int y)
    {
        std::shared_ptr<Camera> cam = cm.get_camera(camera);
        for (auto &entity : registered_entities)
        {
            auto text = cm.texts.at(entity);
            auto font = cm.fonts.at(entity);
			auto shader = cm.shaders.at(entity);

            cm.shaders.at(entity)->use();
            cm.transforms.at(entity)->update(x, y);

            if (cam != nullptr)
            {
                cm.transforms.at(entity)->update_camera(cam->position);
            }
            if (text->center)
            {
                cm.transforms.at(entity)->update(x, y, 1.0f, glm::vec3(-getWidth(entity, cm) * 0.5f, -getHeight(entity, cm) * 0.5f, 0.0f));
            }

            set_shader_uniforms(entity, cm);
            if (!text->hidden)
            {
                text->render(*font, 0.0f, 0.0f, getHeight(entity, cm), shader);
            }
        }
    }
};