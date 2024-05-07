#pragma once
#include "../event.hpp"
#include "../timeless.hpp"
#include "system.hpp"

class MouseInputSystem : public System
{
public:
	Entity camera;

	void register_camera(Entity c)
	{
		camera = c;
	}

	void notify_listener(ComponentManager& cm, MouseEvent* event, Entity entity)
	{
		auto listener = cm.get_mouse_input_listener(entity);
		listener->on_click_handler(event, entity, 0);
	}

	void mouse_click_handler(ComponentManager& cm, MouseEvent* event)
	{
		auto cam = cm.get_camera(camera);

		float highest_z = -999.0f;
		Entity found_entity;
		bool entity_was_found = false;

		glm::vec2 m_pos(event->screen_position.x, event->screen_position.y);

		std::stable_sort(registered_entities.begin(), registered_entities.end(), [&, cm = cm](auto a, auto b) {
			auto trans_a = cm.transforms.at(a);
			auto trans_b = cm.transforms.at(b);
			return trans_a->position.z > trans_b->position.z;
			});

		// for (const auto &[entity, transform] : transforms)
		for (const auto& entity : registered_entities)
		{
			auto transform = cm.get_transform(entity);
			if (transform != nullptr)
			{
				glm::vec3 t_pos = transform->get_position_from_camera();
				//if (transform->isometric) 
				//{
				//	t_pos.x += 128.0f;
				//	t_pos.y -= 64.0f;
				//}
				//double x_pos = (x - y) * t_width / 2;
				//double y_pos = (y + x) * t_height / 2;

				// small hack - if transform width/height (scale) is 1.0f, i.e. for Text, 
				// create a quick new hitbox. TODO - use separate Collider here, don't use transform scale here!
				float w = transform->width;
				float h = transform->height;
				if (w == 1.0f)
				{
					w = 16.0f;
				}
				if (h == 1.0f)
				{
					h = 16.0f;
				}
				if (m_pos.x > (t_pos.x - w) && m_pos.x < (t_pos.x + w) &&
					m_pos.y >(t_pos.y - h) && m_pos.y < (t_pos.y + h))
				{

					if (t_pos.z > highest_z)
					{
						highest_z = t_pos.z;
						found_entity = entity;
						entity_was_found = true;
					}
				}
			}
		}
		if (entity_was_found)
		{
			notify_listener(cm, event, found_entity);
		}

		std::string et = event->eventType;
		event->eventType = "Global" + et;
		for (const auto& entity : registered_entities)
		{
			notify_listener(cm, event, entity);
		}
	}
	void mouse_release_handler(ComponentManager& cm, MouseEvent* event)
	{
		for (const auto& entity : registered_entities)
		{
			notify_listener(cm, event, entity);
		}
	}
	void mouse_move_handler(ComponentManager& cm, MouseEvent* event)
	{
		double normalizedX = (double)event->screen_position.x / TESettings::SCREEN_X;
		double normalizedY = (double)event->screen_position.y / TESettings::SCREEN_Y;
		glm::vec2 m_pos(normalizedX * TESettings::VIEWPORT_X, normalizedY * TESettings::VIEWPORT_Y);

		event->screen_position = m_pos;

		for (const auto& entity : registered_entities)
		{
			notify_listener(cm, event, entity);
		}
	}
	void mouse_scroll_handler(ComponentManager& cm, MouseEvent* event)
	{
		std::stable_sort(registered_entities.begin(), registered_entities.end(), [&, cm = cm](auto a, auto b) {
			auto trans_a = cm.transforms.at(a);
			auto trans_b = cm.transforms.at(b);
			return trans_a->position.z > trans_b->position.z;
		});
		for (const auto& entity : registered_entities)
		{
			notify_listener(cm, event, entity);
		}
	}
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		// ViewportSettings::SCR_VIEWPORT_X += (ViewportSettings::SCR_VIEWPORT_X * yoffset) * 0.5f;
		// ViewportSettings::SCR_VIEWPORT_Y += (ViewportSettings::SCR_VIEWPORT_Y * yoffset) * 0.5f;
	}
};