#pragma once
#include "../components/shader.hpp"
#include "../components/transform.hpp"
#include "system.hpp"

/** This rendering system is used for basic geometry - no textures etc.*/
class GeoRenderingSystem : public System
{
public:
    Entity camera;

    void register_camera(Entity c)
    {
        camera = c;
    }

    void set_shader_transform_uniforms(std::shared_ptr<Shader> shader, std::shared_ptr<Transform> transform)
    {
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(transform->projection));
         glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(transform->model));
         glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE, glm::value_ptr(transform->view));
        // glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
        //glm::mat4 transformMatrix = glm::translate(glm::mat4(1.0f), start - camera_position);

        //glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(transformMatrix));
        //glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
    }

    void render(ComponentManager &cm, int x = TESettings::SCREEN_X, int y = TESettings::SCREEN_Y, float zoom = 1.0)
    {
        // get camera if set
        std::shared_ptr<Camera> cam = cm.get_camera(camera);
        for (auto &entity : registered_entities)
        {
            cm.shaders.at(entity)->use();
            cm.transforms.at(entity)->update(x, y, zoom);
            if (cam != nullptr)
            {
                cm.transforms.at(entity)->update_camera(cam->position);
            }
            cm.geometry.at(entity)->render();
            auto line = cm.geometry.at(entity);
            set_shader_transform_uniforms(cm.shaders.at(entity), cm.transforms.at(entity));
        }
    }
};