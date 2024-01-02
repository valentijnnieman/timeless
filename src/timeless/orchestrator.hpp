#pragma once
#include "timeless/components/transform.hpp"
#include "timeless/components/quad.hpp"
#include "timeless/components/texture.hpp"
#include "timeless/components/shader.hpp"
#include "timeless/components/camera.hpp"
#include "timeless/components/mouse_input_listener.hpp"
#include "timeless/components/movement_controller.hpp"
#include "timeless/components/font.hpp"
#include "timeless/managers/component_manager.hpp"
#include "timeless/managers/window_manager.hpp"
#include "timeless/systems/rendering_system.hpp"
#include "timeless/systems/geo_rendering_system.hpp"
#include "timeless/systems/text_rendering_system.hpp"
#include "timeless/systems/mouse_input_system.hpp"
#include "timeless/systems/movement_system.hpp"
#include "timeless/systems/npc_ai_system.hpp"
#include "timeless/systems/inventory_system.hpp"
#include "timeless/algorithm/graph.hpp"
// #include "timeless/algorithm/ud_graph.hpp"

class Orchestrator
{
private:
    std::unique_ptr<ComponentManager> cm;
    std::unique_ptr<WindowManager> wm;
    std::unique_ptr<GeoRenderingSystem> g_rs;
    std::unique_ptr<RenderingSystem> rs;
    std::unique_ptr<RenderingSystem> ui_rs;
    std::unique_ptr<TextRenderingSystem> t_rs;
    std::unique_ptr<TextRenderingSystem> ui_t_rs;
    std::unique_ptr<MovementSystem> mvs;
    std::unique_ptr<NpcAiSystem> ai_s;
    std::unique_ptr<Grid> grid;
    // std::unique_ptr<Graph> graph;
    std::unique_ptr<InventorySystem> invs;

public:
    void init()
    {
        cm = std::make_unique<ComponentManager>();
        wm = std::make_unique<WindowManager>();
        g_rs = std::make_unique<GeoRenderingSystem>();
        rs = std::make_unique<RenderingSystem>();
        ui_rs = std::make_unique<RenderingSystem>();
        t_rs = std::make_unique<TextRenderingSystem>();
        ui_t_rs = std::make_unique<TextRenderingSystem>();
        mvs = std::make_unique<MovementSystem>();
        ai_s = std::make_unique<NpcAiSystem>();
        grid = std::make_unique<Grid>(Grid());
        // graph = std::make_unique<Graph>(new_graph);
        invs = std::make_unique<InventorySystem>(InventorySystem());
    }

    // void set_graph(Graph g)
    // {
    //     graph = std::make_unique<Graph>(g);
    // }

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

    void remove_entity(Entity entity)
    {
        cm->remove_entity(entity);

        MouseInputSystem::remove_entity(entity);
        mvs->remove_entity(entity);
        ai_s->remove_entity(entity);
        rs->remove_entity(entity);
        ui_rs->remove_entity(entity);
        t_rs->remove_entity(entity);
        ui_t_rs->remove_entity(entity);
    }

    template <typename T>
    void add_component(Entity entity, std::shared_ptr<T> comp)
    {
        cm->add_component(entity, comp);
    }

    /** These separately defined methods are so we can handle things like
     * registering entities/components with different systems etc.
     * It could have been done with templates and a bitfield type signature
     * in the entity id's, but I opted to not spend the time on that and cut
     * corners here - but this would probably be a worthwile improvement.
     */
    void add_component(Entity entity, MouseInputListener *mouse_input, bool add_transform = true, bool is_ui = false)
    {
        cm->add_component(entity, mouse_input);
        MouseInputSystem::register_listener(entity, get_mouse_input_listener(entity));
        if (add_transform)
        {
            if (is_ui)
            {
                MouseInputSystem::register_ui_transform(entity, get_transform(entity));
            }
            else
            {
                MouseInputSystem::register_transform(entity, get_transform(entity));
            }
        }
    }
    void add_component(Entity entity, Node *node)
    {
        cm->add_component(entity, node);
        grid->register_entity(entity);
        // graph->register_entity(entity);
    }
    void add_component(Entity entity, Behaviour *behaviour)
    {
        cm->add_component(entity, behaviour);
        ai_s->register_entity(entity);
    }
    void add_component(Entity entity, MovementController *movement, bool is_camera = false)
    {
        cm->add_component(entity, movement);
        if (is_camera)
        {
            mvs->register_camera(entity);
        }
        else
        {
            mvs->register_entity(entity);
        }
    }

    void add_component(Entity entity, Camera *camera, bool ui = false)
    {
        cm->add_component(entity, camera);
        if (ui)
        {
            ui_rs->register_camera(entity);
            ui_t_rs->register_camera(entity);
        }
        else
        {
            g_rs->register_camera(entity);
            rs->register_camera(entity);
            t_rs->register_camera(entity);
        }
    }
    void add_component(Entity entity, Sprite *sprite, bool ui = false)
    {
        cm->add_component(entity, sprite);
        if (ui)
        {
            ui_rs->register_entity(entity);
        }
        else
        {
            rs->register_entity(entity);
        }
    }
    void add_component(Entity entity, Text *text, bool ui = false)
    {
        cm->add_component(entity, text);
        if (ui)
        {
            ui_t_rs->register_entity(entity);
        }
        else
        {
            t_rs->register_entity(entity);
        }
    }
    void add_component(Entity entity, Line *line)
    {
        cm->add_component(entity, line);
        g_rs->register_entity(entity);
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
    std::shared_ptr<Behaviour> get_behaviour(Entity entity)
    {
        return cm->behaviours.at(entity);
    }

    /** TODO: return shared_ptr instead of ref to unique_ptr? refs to smart pointers
     * defeat the purpose of smart pointers - although it's not likely our system
     * and manager pointers go out of scope before the program finishes, probably
     * better to do something else here.
     */
    std::unique_ptr<ComponentManager> &get_component_manager()
    {
        return cm;
    }
    std::unique_ptr<Grid> &get_grid()
    {
        return grid;
    }
    // std::unique_ptr<Graph> &get_graph()
    // {
    //     return graph;
    // }
    std::unique_ptr<NpcAiSystem> &get_npc_ai_system()
    {
        return ai_s;
    }
    std::unique_ptr<TextRenderingSystem> &get_ui_text_rendering_system()
    {
        return ui_t_rs;
    }
    std::unique_ptr<InventorySystem> &get_inventory_system()
    {
        return invs;
    }
    std::unique_ptr<WindowManager> &get_window_manager()
    {
        return wm;
    }

    void loop()
    {
        wm->loop(*cm, *g_rs, *rs, *ui_rs, *t_rs, *ui_t_rs, *mvs, *ai_s);
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