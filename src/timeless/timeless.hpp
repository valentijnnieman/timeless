#pragma once
#include "timeless/components/transform.hpp"
#include "timeless/components/quad.hpp"
#include "timeless/components/texture.hpp"
#include "timeless/components/shader.hpp"
#include "timeless/components/camera.hpp"
#include "timeless/components/mouse_input_listener.hpp"
#include "timeless/components/event_listener.hpp"
#include "timeless/components/movement_controller.hpp"
#include "timeless/components/keyboard_input_listener.hpp"
#include "timeless/components/font.hpp"
#include "timeless/event.hpp"
#include "timeless/managers/component_manager.hpp"
#include "timeless/managers/window_manager.hpp"
#include "timeless/systems/rendering_system.hpp"
#include "timeless/systems/geo_rendering_system.hpp"
#include "timeless/systems/text_rendering_system.hpp"
#include "timeless/systems/mouse_input_system.hpp"
#include "timeless/systems/movement_system.hpp"
#include "timeless/systems/keyboard_input_system.hpp"
#include "timeless/systems/npc_ai_system.hpp"
#include "timeless/systems/inventory_system.hpp"
#include "timeless/systems/event_system.hpp"
#include "timeless/systems/animation_system.hpp"
#include "timeless/systems/sound_system.hpp"
#include "timeless/algorithm/graph.hpp"
#include "timeless/systems/system.hpp"

namespace TE
{
    std::map<std::string, std::shared_ptr<System>> systems;

    std::shared_ptr<ComponentManager> cm;
    std::shared_ptr<MouseInputSystem> mis;
    std::shared_ptr<WindowManager> wm;
    std::shared_ptr<Grid> grid;

    void init()
    {
        cm = std::make_shared<ComponentManager>();
        mis = std::make_shared<MouseInputSystem>();
        wm = std::make_shared<WindowManager>(cm, mis);

        grid = std::make_shared<Grid>(Grid());
    }

    template <typename T>
    void create_system(const std::string& key, T* system)
    {
        systems.insert({ key, std::shared_ptr<T>(system) });
    }

    template <typename T>
    std::shared_ptr<T> get_system(const std::string &key) {
      return std::dynamic_pointer_cast<T>(systems[key]);
    }

    template <typename T>
    std::shared_ptr<T> get_component(Entity entity) {
      return cm->get_component<T>(entity);
    }

    /** Finalizes the Grid by calculating near neighbours for every Node.
     * This needs to be called seperately and after init(), so components
     * can be created for the grid first (see main.cpp in example game)
     */
    void calculate_grid()
    {
        grid->calculate_nodes(*cm);
    }

    template <typename T>
    void add_component(Entity entity, T *comp)
    {
        cm->add_component(entity, comp);
    }

    template <typename T>
    void add_component(Entity entity, std::shared_ptr<T> comp)
    {
        cm->add_component(entity, comp);
    }

    void remove_entity(Entity entity)
    {
        cm->remove_entity(entity);

        mis->remove_entity(entity);
        for (auto [key, system] : systems)
        {
            system->remove_entity(entity);
        }
    }


    /** These separately defined methods are so we can handle things like
     * registering entities/components with default systems like the mouse input system and the grid system.
     */
    template<> void add_component<MouseInputListener<MouseEvent>>(Entity entity, MouseInputListener<MouseEvent> *mouse_input)
    {
        cm->add_component(entity, mouse_input);
        mis->register_entity(entity);
    }
    template<> void add_component<MouseInputListener<MouseMoveEvent>>(Entity entity, MouseInputListener<MouseMoveEvent> *mouse_input)
    {
        cm->add_component(entity, mouse_input);
        mis->register_move_entity(entity);
    }
    template<> void add_component<Node>(Entity entity, Node *node)
    {
        cm->add_component(entity, node);
        grid->register_entity(entity);
    }

    /** TODO: return shared_ptr instead of ref to unique_ptr? refs to smart pointers
     * defeat the purpose of smart pointers - although it's not likely our system
     * and manager pointers go out of scope before the program finishes, probably
     * better to do something else here.
     */
    std::shared_ptr<ComponentManager> get_component_manager()
    {
        return cm;
    }
    std::shared_ptr<Grid> get_grid()
    {
        return grid;
    }
    std::shared_ptr<MouseInputSystem> get_mouse_input_system()
    {
        return mis;
    }
    std::shared_ptr<WindowManager> get_window_manager()
    {
        return wm;
    }

    void loop(std::function<void(GLFWwindow *window, ComponentManager &cm, WindowManager &wm)> loop_func)
    {
        loop_func(wm->window, *cm, *wm);
    }
    void quit()
    {
        wm->running = false;
    }
    void generate_collider_levels(int start_x1, int start_x2, int start_y1, int start_y2, int n)
    {
        for (int i = 0; i < n; i++)
        {
            NodeCollider n1(start_x1 - i, start_x2 - i, start_y1 - i, start_y2 - i);
            grid->colliders.push_back(n1);
        }
    }

    // helper function that checks if mouse event position overlaps with
    // transform position of an entity. Can be used to determine if an entity was clicked on, for example.
    bool clicked_on(MouseEvent* event, Entity entity, float zoom = 1.0f)
    {
        auto transform = TE::get_component<Transform>(entity);
        glm::vec2 pos = glm::vec2(transform->get_centered_position_from_camera().x / zoom, transform->get_centered_position_from_camera().y / zoom);

        return ((event->screen_position.x > pos.x - transform->width && event->screen_position.x < pos.x + transform->width)
          &&
          (event->screen_position.y > pos.y - transform->height && event->screen_position.y < pos.y + transform->height)
          );
    }
};
