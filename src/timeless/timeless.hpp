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
    std::unique_ptr<WindowManager> wm;
    std::unique_ptr<Grid> grid;

    void init()
    {
        cm = std::make_shared<ComponentManager>();
        mis = std::make_shared<MouseInputSystem>();
        wm = std::make_unique<WindowManager>(cm, mis);

        grid = std::make_unique<Grid>(Grid());
    }

    template <typename T>
    void create_system(const std::string& key, T* system)
    {
        systems.insert({ key, std::shared_ptr<T>(system) });
    }


    template <typename T>
    std::shared_ptr<T> get_system(const std::string& key)
    {
        return std::dynamic_pointer_cast<T>(systems[key]);
    }

    std::shared_ptr<Quad> get_quad(Entity entity)
    {
        return cm->quads.at(entity);
    }
    std::shared_ptr<Transform> get_transform(Entity entity)
    {
        return cm->transforms.at(entity);
    }
    std::shared_ptr<Texture> get_texture(Entity entity)
    {
        return cm->textures.at(entity);
    }
    std::shared_ptr<Shader> get_shader(Entity entity)
    {
        return cm->shaders.at(entity);
    }
    std::shared_ptr<Camera> get_camera(Entity entity)
    {
        return cm->cameras.at(entity);
    }
    std::shared_ptr<Sprite> get_sprite(Entity entity)
    {
        return cm->sprites.at(entity);
    }
    std::shared_ptr<Node> get_node(Entity entity)
    {
        return cm->nodes.at(entity);
    }
    std::shared_ptr<Font> get_font(Entity entity)
    {
        return cm->fonts.at(entity);
    }
    std::shared_ptr<Text> get_text(Entity entity)
    {
        return cm->texts.at(entity);
    }
    std::shared_ptr<MouseInputListener> get_mouse_input_listener(Entity entity)
    {
        return cm->mouse_input_listeners.at(entity);
    }
    std::shared_ptr<EventListener<Event>> get_event_listener(Entity entity)
    {
        return cm->event_listeners.at(entity);
    }
    std::shared_ptr<Behaviour> get_behaviour(Entity entity)
    {
        return cm->behaviours.at(entity);
    }
    std::shared_ptr<Animation> get_animation(Entity entity)
    {
        return cm->animations.at(entity);
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

    template <typename T>
    void remove_component(Entity entity, T *comp)
    {
        cm->remove_component(entity, comp);
    }

    template <typename T>
    void remove_component(Entity entity, std::shared_ptr<T> comp)
    {
        cm->remove_component(entity, comp);
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
    void add_component(Entity entity, MouseInputListener *mouse_input, bool add_transform = true, bool is_ui = false)
    {
        cm->add_component(entity, mouse_input);
        mis->register_entity(entity);
    }
    void add_component(Entity entity, Node *node)
    {
        cm->add_component(entity, node);
        grid->register_entity(entity);
    }

    /** TODO: return shared_ptr instead of ref to unique_ptr? refs to smart pointers
     * defeat the purpose of smart pointers - although it's not likely our system
     * and manager pointers go out of scope before the program finishes, probably
     * better to do something else here.
     */
    std::shared_ptr<ComponentManager> &get_component_manager()
    {
        return cm;
    }
    std::unique_ptr<Grid> &get_grid()
    {
        return grid;
    }
    std::shared_ptr<MouseInputSystem> &get_mouse_input_system()
    {
        return mis;
    }
    std::unique_ptr<WindowManager> &get_window_manager()
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
};