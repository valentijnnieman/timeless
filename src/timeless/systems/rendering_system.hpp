#pragma once
#include "../components/shader.hpp"
#include "../components/transform.hpp"
#include <string.h>

class RenderingSystem
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

    void set_shader_transform_uniforms(std::shared_ptr<Shader> shader, std::shared_ptr<Transform> transform)
    {
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(transform->projection));
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(transform->model));
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE, glm::value_ptr(transform->view));
    }
    void set_shader_sprite_uniforms(std::shared_ptr<Shader> shader, std::shared_ptr<Sprite> sprite)
    {
        glUniform1i(glGetUniformLocation(shader->ID, "texture1"), 0);
        glUniform1f(glGetUniformLocation(shader->ID, "index"), sprite->index);
        glUniform4fv(glGetUniformLocation(shader->ID, "highlightColor"), 1, glm::value_ptr(sprite->color));
        glUniform2fv(glGetUniformLocation(shader->ID, "spriteSheetSize"), 1, glm::value_ptr(sprite->spriteSheetSize));
        glUniform2fv(glGetUniformLocation(shader->ID, "spriteSize"), 1, glm::value_ptr(sprite->spriteSize));
    }

    void render(ComponentManager &cm, int x, int y, float zoom = 1.0)
    {
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // get camera if set
        std::shared_ptr<Camera> cam = cm.get_camera(camera);

		std::stable_sort(registered_entities.begin(), registered_entities.end(), [&, cm = cm](auto a, auto b) { 
            auto trans_a = cm.transforms.at(a);
            auto trans_b = cm.transforms.at(b);
            return (trans_a->position.y - trans_a->position.z)  < (trans_b->position.y - trans_b->position.z);
            });

        for (auto &entity : registered_entities)
        {
            cm.shaders.at(entity)->use();
            cm.transforms.at(entity)->update(x, y, zoom);
            if (cam != nullptr)
            {
                cm.transforms.at(entity)->update_camera(cam->position);
            }
            cm.quads.at(entity)->render();
            cm.textures.at(entity)->render();

            set_shader_transform_uniforms(cm.shaders.at(entity), cm.transforms.at(entity));
            set_shader_sprite_uniforms(cm.shaders.at(entity), cm.sprites.at(entity));
            auto sprite = cm.sprites.at(entity);
            if (!sprite->hidden)
            {
                sprite->update();
                sprite->render();
            }
        }
    }
};