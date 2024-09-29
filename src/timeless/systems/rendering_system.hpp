#pragma once
#include "../components/shader.hpp"
#include "../components/transform.hpp"
#include "system.hpp"
#include <string.h>

class RenderingSystem : public System
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
	}
	void set_shader_sprite_uniforms(std::shared_ptr<Shader> shader, std::shared_ptr<Sprite> sprite, std::shared_ptr<Transform> transform, int tick = 0, glm::vec3 cam_position = glm::vec3(0.0))
	{
		glUniform1i(glGetUniformLocation(shader->ID, "texture1"), 0);
		glUniform1f(glGetUniformLocation(shader->ID, "index"), sprite->index);
		glUniform4fv(glGetUniformLocation(shader->ID, "highlightColor"), 1, glm::value_ptr(sprite->color));
		glUniform2fv(glGetUniformLocation(shader->ID, "spriteSheetSize"), 1, glm::value_ptr(sprite->spriteSheetSize));
		glUniform2fv(glGetUniformLocation(shader->ID, "spriteSize"), 1, glm::value_ptr(sprite->spriteSize));
        glUniform1f(glGetUniformLocation(shader->ID, "time"), glfwGetTime());
        glUniform1f(glGetUniformLocation(shader->ID, "tick"), tick);
	}

	void rotate_all_z(ComponentManager& cm, float z)
	{

		for (auto& entity : registered_entities)
		{
			auto transform = cm.transforms.at(entity);
			//transform->position.x += 42.0f;
			//transform->position.x += transform->width * 0.75f;
			//transform->position.y -= 8.0f;
			//transform->position.y += transform->height * 0.5f;
			transform->rotate(glm::vec3(0.0f, 0.0f, glm::radians(z)));
		}
	}

	void reset_shadow(ComponentManager& cm, float z)
	{

		for (auto& entity : registered_entities)
		{
			auto transform = cm.transforms.at(entity);
			transform->rotate(glm::vec3(0.0f, 0.0f, glm::radians(z)));
			//transform->position.x -= 42.0f;
			//transform->position.x -= transform->width * 0.75f;
			//transform->position.y += 8.0f;
			//transform->position.y -= transform->height * 0.5f;
		}
	}

	void render(ComponentManager& cm, int x, int y, float zoom = 1.0, int tick = 0)
	{ 
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// get camera if set
		std::shared_ptr<Camera> cam = cm.get_camera(camera);

		float t_width = 256.0f;
		float t_height = 128.0f;

		// std::stable_sort(registered_entities.begin(), registered_entities.end(), [&, cm = cm](auto a, auto b) {
		// 	auto trans_a = cm.transforms.at(a);
		// 	auto trans_b = cm.transforms.at(b);
		//
		// 	double a_h = trans_a->height / t_height;
		// 	double b_h = trans_b->height / t_height;
		//
		// 	double a_w = trans_a->width / t_width;
		// 	double b_w = trans_b->width / t_width;
		//
		// 	float a_x_pos = trans_a->grid_x;
		// 	float a_y_pos = trans_a->grid_y;
		// 	float b_x_pos = trans_b->grid_x;
		// 	float b_y_pos = trans_b->grid_y;
		//
		// 	return (a_y_pos + a_x_pos) < (b_y_pos + b_x_pos);
		// 	});
		//
		// std::stable_sort(registered_entities.begin(), registered_entities.end(), [&, cm = cm](auto a, auto b) {
		// 	auto trans_a = cm.transforms.at(a);
		// 	auto trans_b = cm.transforms.at(b);
		// 	return trans_a->position.z < trans_b->position.z;
		// 	});


		for (auto& entity : registered_entities)
		{
			cm.shaders.at(entity)->use();
			cm.transforms.at(entity)->update(x, y, zoom);
			if (cam != nullptr)
			{
				cm.transforms.at(entity)->update_camera(cam->get_position());
			}
			set_shader_transform_uniforms(cm.shaders.at(entity), cm.transforms.at(entity));
			set_shader_sprite_uniforms(cm.shaders.at(entity), cm.sprites.at(entity), cm.transforms.at(entity), tick, cam->get_position());
			cm.quads.at(entity)->render();
			cm.textures.at(entity)->render();

			auto sprite = cm.sprites.at(entity);
			if (!sprite->hidden)
			{
				sprite->update();
				sprite->render();
			}
		}
	}
};
