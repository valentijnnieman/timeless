#pragma once
#include "../components/font.hpp"
#include "../components/text.hpp"

class TextRenderingSystem
{
public:
    std::vector<Entity> registered_entities;
    Entity camera;

    void register_entity(Entity entity)
    {
        registered_entities.push_back(entity);
    }
    void remove_entity(Entity entity)
    {
        if (!registered_entities.empty())
        {
            auto found = std::find_if(registered_entities.begin(), registered_entities.end(), [&](auto &e)
                                      { return e == entity; });
            if (found != registered_entities.end())
            {
                registered_entities.erase(found);
            }
        }
    }

    void register_camera(Entity c)
    {
        camera = c;
    }

    float getWidth(Entity entity, ComponentManager &cm)
    {
        auto font = cm.fonts.at(entity);
        auto text = cm.texts.at(entity);

        float width;
        // Get width of string
        std::string::const_iterator c;
        for (c = text->text.begin(); c != text->text.end(); c++)
        {
            Glyph glyph = font->glyphs.at(*c);
            width += glyph.advance >> 6;
        }
        return width;
    }
    float get_width(std::shared_ptr<Font> font, std::shared_ptr<Text> text)
    {
        float width;
        // Get width of string
        std::string::const_iterator c;
        for (c = text->text.begin(); c != text->text.end(); c++)
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

        float height;
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
        // glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f);

        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(transform->projection));
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(transform->model));
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE, glm::value_ptr(transform->view));

        glUniform3f(glGetUniformLocation(shader->ID, "textColor"), 0.0f, 0.0f, 0.0f);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(font->VAO);
    }

    void render(ComponentManager &cm)
    {
        std::shared_ptr<Camera> cam = cm.get_camera(camera);
        for (auto &entity : registered_entities)
        {
            cm.shaders.at(entity)->use();
            cm.transforms.at(entity)->update();
            if (cam != nullptr)
            {
                cm.transforms.at(entity)->update_camera(cam->position);
            }
            set_shader_uniforms(entity, cm);
            auto font = cm.fonts.at(entity);
            cm.texts.at(entity)->render(*font, cm.transforms.at(entity)->get_position_from_camera().x, cm.transforms.at(entity)->get_position_from_camera().y, getHeight(entity, cm));
        }
    }
};