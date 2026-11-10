#pragma once

// ===========================================================================
// timeless/core.hpp — the engine facade.
//
// Defines the TE:: namespace (init/loop/cleanup, the global handles cm/tm/mis/
// wm/grid, component & system registration, and the hit-testing helpers) plus
// only the headers that this facade code actually references.
//
// Include THIS plus the specific component/system headers your game uses, e.g.
//     #include "timeless/core.hpp"
//     #include "timeless/systems/rendering_system.hpp"
//     #include "timeless/components/text.hpp"
// For the historical "pull in everything" behaviour use <timeless/all.hpp>
// (or <timeless/timeless.hpp>, which forwards to it).
// ===========================================================================

// --- C++ standard library used by the facade below ---
#include <map>
#include <memory>
#include <string>
#include <functional>
#include <iostream>
#include <algorithm> // std::swap, std::max/min in the slab test
#include <cmath>     // std::abs
#include <limits>    // infinity sentinels in the slab test

// --- glm used by the hit-testing helpers ---
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::unProject
#include <glm/gtx/quaternion.hpp>       // glm::quat, glm::inverse

// --- engine headers whose symbols the facade references directly ---
#include "timeless/entity.hpp"          // Entity, active_entities
#include "timeless/settings.hpp"        // TESettings::VIEWPORT_*, ZOOM, WINDOW_*
#include "timeless/event.hpp"           // MouseEvent, MouseMoveEvent
#include "timeless/timer.hpp"           // TimerManager
#include "timeless/managers/component_manager.hpp" // ComponentManager
#include "timeless/managers/window_manager.hpp"    // WindowManager
#include "timeless/systems/system.hpp"             // System
#include "timeless/systems/mouse_input_system.hpp" // MouseInputSystem
#include "timeless/algorithm/graph.hpp"            // Grid, Node
#include "timeless/components/transform.hpp"       // Transform
#include "timeless/components/camera.hpp"          // Camera
#include "timeless/components/animation.hpp"       // Animation
#include "timeless/components/model.hpp"           // Model bounds for picking
#include "timeless/components/skeletal_animation.hpp" // posed bounds for picking
#include "timeless/components/mouse_input_listener.hpp" // add_component<MouseInputListener<...>>

namespace TE
{
    inline std::map<std::string, std::shared_ptr<System>> systems;

    inline std::shared_ptr<ComponentManager> cm;
    inline std::shared_ptr<TimerManager> tm;
    inline std::shared_ptr<MouseInputSystem> mis;
    inline std::shared_ptr<WindowManager> wm;
    inline std::shared_ptr<Grid> grid;

    void init();

    void cleanup();

    template <typename T>
    inline void create_system(const std::string& key, T* system)
    {
        systems.insert_or_assign(key, std::shared_ptr<T>(system));
    }

    void remove_system(const std::string& key);

    void print_systems();

    // Look up a system by key, or nullptr if there's none registered under it.
    // Uses find() rather than operator[] on purpose: operator[] would INSERT a
    // null shared_ptr for a missing key, and that null then gets dereferenced by
    // every loop that iterates `systems` (cleanup/purge, remove_entity), which
    // segfaults. Probing for an optional system (e.g. an absent SoundSystem)
    // each frame must therefore not mutate the map.
    template <typename T>
    inline std::shared_ptr<T> get_system(const std::string &key) {
      auto it = systems.find(key);
      if (it == systems.end())
        return nullptr;
      return std::dynamic_pointer_cast<T>(it->second);
    }

    template <typename T>
    const inline std::shared_ptr<T> get_component(Entity entity) {
      return cm->get_component<T>(entity);
    }

    /** Finalizes the Grid by calculating near neighbours for every Node.
     * This needs to be called seperately and after init(), so components
     * can be created for the grid first (see main.cpp in example game)
     */
    inline void calculate_grid()
    {
        grid->calculate_nodes(cm->nodes);
    }

    inline void clear_grid()
    {
        grid->purge();
    }

    template <typename T>
    inline void add_component(Entity entity, T *comp)
    {
        cm->add_component(entity, comp);
    }

    template <typename T>
    inline void add_component(Entity entity, std::shared_ptr<T> comp)
    {
        cm->add_component(entity, comp);
    }

    void remove_entity(Entity entity, bool destroy = true);


    /** These separately defined methods are so we can handle things like
     * registering entities/components with default systems like the mouse input system and the grid system.
     */
    template<> inline void add_component<MouseInputListener<MouseEvent>>(Entity entity, MouseInputListener<MouseEvent> *mouse_input)
    {
        cm->add_component(entity, mouse_input);
        mis->register_entity(entity);
    }
    template<> inline void add_component<MouseInputListener<MouseMoveEvent>>(Entity entity, MouseInputListener<MouseMoveEvent> *mouse_input)
    {
        cm->add_component(entity, mouse_input);
        mis->register_move_entity(entity);
    }
    template<> inline void add_component<Node>(Entity entity, Node *node)
    {
        cm->add_component(entity, node);
        // grid->register_entity(entity);
    }
    template<> inline void add_component<Node>(Entity entity, std::shared_ptr<Node> node)
    {
        cm->add_component(entity, node);
        // grid->register_entity(entity);
    }

    inline std::shared_ptr<ComponentManager> get_component_manager()
    {
        return cm;
    }
    inline std::shared_ptr<TimerManager> get_timer_manager()
    {
        return tm;
    }
    inline std::shared_ptr<Grid> get_grid()
    {
        return grid;
    }
    inline std::shared_ptr<MouseInputSystem> get_mouse_input_system()
    {
        return mis;
    }
    inline std::shared_ptr<WindowManager> get_window_manager()
    {
        return wm;
    }

    inline void loop(std::function<void(WindowManager &wm, ComponentManager &cm)> loop_func)
    {
        loop_func(*wm, *cm);
    }
    inline void quit()
    {
        wm->running = false;
    }

    // helper function that checks if mouse event position overlaps with
    // transform position of an entity. Can be used to determine if an entity was clicked on, for example.
    bool clicked_on(MouseEvent* event, Entity entity, float zoom = 1.0f);
    // Slab test of an infinite line (not a half-ray — callers rely on hits
    // behind the origin, because the orthographic path below starts the line at
    // the ortho near plane, which sits 10000 units *behind* the camera).
    //
    // Axis-by-axis rather than the classic 1/dir form: the orthographic camera
    // produces direction components that are exactly 0, and dividing by those
    // yields inf (or 0/0 = NaN when the origin lands exactly on a slab face).
    // Handling the zero case explicitly keeps degenerate axes exact.
    bool intersect_ray_aabb(const glm::vec3 &origin, const glm::vec3 &dir,
                            const glm::vec3 &aabb_min, const glm::vec3 &aabb_max);

    // World-space box to pick against. Entities carrying a 3D Model are picked
    // against the model's real bounds pushed through transform->model — the very
    // matrix RenderingSystem draws them with — so the hit area covers the whole
    // model (full height included) rather than a flat quad at its origin.
    // Sprites and anything without model bounds keep the old width/height box.
    // Model-space bounds of an animated model in its *current* pose: each
    // bone's bind-space box (Model::bone_aabb_*) pushed through the same pose
    // matrix the renderer uploads as boneMatrices[i]. The bind-pose box alone
    // can sit well off the drawn figure, since clips move and bend bones.
    // Returns false when there is nothing to pose (no SkeletalAnimation, so
    // the renderer doesn't skin either), leaving the bind box to the caller.
    bool posed_local_aabb(Entity entity, const std::shared_ptr<Model> &model,
                          glm::vec3 &out_min, glm::vec3 &out_max);

    void picking_aabb(Entity entity, const std::shared_ptr<Transform>& transform,
                      glm::vec3 &out_min, glm::vec3 &out_max);

    // Upright figures (Transform::pick_upright) are picked against a
    // screen-space rectangle instead of the world box. The world-axis box
    // around a figure turned 45 degrees is much wider than the figure, and the
    // isometric camera projects the front of its ground footprint below the
    // feet, so clicks and hovers under a citizen still hit. The rectangle runs
    // from the projected feet to the projected head, as wide as the box's
    // narrower horizontal extent. Needs real model bounds: the flat fallback
    // box has no height.
    bool picks_upright(Entity entity, const std::shared_ptr<Transform> &transform);

    bool hit_upright(float win_x, float win_y, Entity entity,
                     const std::shared_ptr<Transform> &transform,
                     const glm::mat4 &view, const glm::mat4 &proj,
                     const glm::vec4 &viewport);

    bool clicked_on_perspective(MouseEvent* event, Entity entity, float zoom = 1.0f);

    bool hovered_over(MouseMoveEvent *event, Entity entity, float zoom = 1.0f);

    bool hovered_over_perspective(MouseMoveEvent *event, Entity entity,
                                  float zoom = 1.0f);
};
