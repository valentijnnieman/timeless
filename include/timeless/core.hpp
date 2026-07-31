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
#include "timeless/components/mouse_input_listener.hpp" // add_component<MouseInputListener<...>>

namespace TE
{
    inline std::map<std::string, std::shared_ptr<System>> systems;

    inline std::shared_ptr<ComponentManager> cm;
    inline std::shared_ptr<TimerManager> tm;
    inline std::shared_ptr<MouseInputSystem> mis;
    inline std::shared_ptr<WindowManager> wm;
    inline std::shared_ptr<Grid> grid;

    inline void init()
    {
        cm = std::make_shared<ComponentManager>();
        tm = std::make_shared<TimerManager>();
        mis = std::make_shared<MouseInputSystem>();
        wm = std::make_shared<WindowManager>(cm, mis);

        grid = std::make_shared<Grid>(Grid());
    }

    inline void cleanup()
    {
        std::cout << "TE Cleanup called" << std::endl;

        for (const auto& [key, system] : systems)
        {
            if (system)
                system->purge();
        }

        // mis.reset();

        cm->cleanup();
        cm.reset();

        grid.reset();

        wm->cleanup();
        wm.reset();

        std::cout << "TE Cleanup finished" << std::endl;
    }

    template <typename T>
    inline void create_system(const std::string& key, T* system)
    {
        systems.insert_or_assign(key, std::shared_ptr<T>(system));
    }

    inline void remove_system(const std::string& key)
    {
      systems[key]->purge();
      systems.erase(key);
    }

    inline void print_systems()
    {
      for (const auto& [key, system] : systems)
      {
          std::cout << "System Key: " << key << std::endl;
      }
    }

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

    inline void remove_entity(Entity entity, bool destroy = true)
    {
        if (!active_entities.contains(entity)) return;
        mis->remove_entity(entity);
        mis->remove_move_entity(entity);
        for (const auto& [key, system] : systems)
        {
            if (system)
                system->remove_entity(entity);
        }
        cm->remove_entity(entity, destroy);
    }


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
    inline bool clicked_on(MouseEvent* event, Entity entity, float zoom = 1.0f)
    {
      auto transform = TE::get_component<Transform>(entity);
      if(transform != nullptr) {
          float w = transform->width / zoom;
          float h = transform->height / zoom;
          glm::vec2 pos = glm::vec2(transform->get_centered_position_from_camera().x / zoom, transform->get_centered_position_from_camera().y / zoom);

          glm::vec3 mouse_pos(event->screen_position, 0.0f); // Promote to vec3 if needed
          // Inverse rotation
          glm::quat inv_rot = glm::inverse(glm::normalize(transform->camera_rotation));

          // Rotate mouse position
          // glm::vec3 rotated_mouse = inv_rot * mouse_pos;
          glm::vec3 rotated_mouse = mouse_pos;

          return ((rotated_mouse.x > pos.x - w && 
                  rotated_mouse.x < pos.x + w) &&
                  (rotated_mouse.y > pos.y - h && 
                  rotated_mouse.y < pos.y + h)
            );
      }
      return false;
    }
    // Slab test of an infinite line (not a half-ray — callers rely on hits
    // behind the origin, because the orthographic path below starts the line at
    // the ortho near plane, which sits 10000 units *behind* the camera).
    //
    // Axis-by-axis rather than the classic 1/dir form: the orthographic camera
    // produces direction components that are exactly 0, and dividing by those
    // yields inf (or 0/0 = NaN when the origin lands exactly on a slab face).
    // Handling the zero case explicitly keeps degenerate axes exact.
    inline bool intersect_ray_aabb(const glm::vec3 &origin, const glm::vec3 &dir,
                                  const glm::vec3 &aabb_min, const glm::vec3 &aabb_max) {
      float tmin = -std::numeric_limits<float>::infinity();
      float tmax = std::numeric_limits<float>::infinity();

      for (int i = 0; i < 3; ++i) {
        if (std::abs(dir[i]) < 1e-8f) {
          // Line is parallel to this pair of slabs: it either sits between them
          // for its whole length or misses entirely.
          if (origin[i] < aabb_min[i] || origin[i] > aabb_max[i])
            return false;
          continue;
        }
        float inv = 1.0f / dir[i];
        float t1 = (aabb_min[i] - origin[i]) * inv;
        float t2 = (aabb_max[i] - origin[i]) * inv;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
        if (tmin > tmax)
          return false;
      }
      return true;
    }

    // World-space box to pick against. Entities carrying a 3D Model are picked
    // against the model's real bounds pushed through transform->model — the very
    // matrix RenderingSystem draws them with — so the hit area covers the whole
    // model (full height included) rather than a flat quad at its origin.
    // Sprites and anything without model bounds keep the old width/height box.
    inline void picking_aabb(Entity entity, const std::shared_ptr<Transform>& transform,
                             glm::vec3 &out_min, glm::vec3 &out_max) {
      auto model = TE::get_component<Model>(entity);
      if (model != nullptr && model->has_local_aabb) {
        glm::vec3 lo = model->local_aabb_min;
        glm::vec3 hi = model->local_aabb_max;

        // Slack is applied in the model's own space so it scales and rotates
        // with the model instead of skewing the world-space box.
        glm::vec3 center = (lo + hi) * 0.5f;
        glm::vec3 extent = (hi - lo) * 0.5f *
                           glm::vec3(transform->hit_scale_x, transform->hit_scale_y,
                                     transform->hit_scale_z);
        lo = center - extent;
        hi = center + extent;

        // Bound the 8 transformed corners: the model matrix carries the
        // isometric 45° spin and per-axis scale, so the local box is not
        // axis-aligned once it lands in the world.
        out_min = glm::vec3(std::numeric_limits<float>::max());
        out_max = glm::vec3(std::numeric_limits<float>::lowest());
        for (int i = 0; i < 8; ++i) {
          glm::vec3 corner((i & 1) ? hi.x : lo.x,
                           (i & 2) ? hi.y : lo.y,
                           (i & 4) ? hi.z : lo.z);
          glm::vec3 world = glm::vec3(transform->model * glm::vec4(corner, 1.0f));
          out_min = glm::min(out_min, world);
          out_max = glm::max(out_max, world);
        }
        return;
      }

      // peek_position(), not get_position(): the latter pops a frame off the
      // transform's animation queue, so hit-testing would fast-forward every
      // animated entity the mouse passes over.
      glm::vec3 box_center = transform->peek_position();
      float w = transform->width;
      float h = transform->height;
      if (transform->width == 0.0f && transform->height == 0.0f) {
        w = transform->scale.x * transform->hit_scale_x;
        h = transform->scale.y * transform->hit_scale_y;
      }
      out_min = box_center - glm::vec3(w, h, 0.0f);
      out_max = box_center + glm::vec3(w, h, 0.0f);
    }

    inline bool clicked_on_perspective(MouseEvent* event, Entity entity, float zoom = 1.0f)
    {
    auto transform = TE::get_component<Transform>(entity);
    auto animation = TE::get_component<Animation>(entity);
    if(animation != nullptr && animation->playing) {
      transform = animation->root->transform;
    }
    if(transform != nullptr) {
        // Get camera matrices and viewport
        auto camera = transform->camera;
        if(camera != nullptr) {
          glm::mat4 view = camera->get_view_matrix();
          glm::mat4 proj = camera->get_projection_matrix(TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y, TESettings::ZOOM);
          // glm::mat4 proj = camera->get_projection_matrix();

          // glm::vec4 viewport = camera->get_viewport(); // (x, y, width, height)
          glm::vec4 viewport = glm::vec4(0, 0, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);

          // Scale raw window pixels to design-resolution (viewport) pixels before
          // unprojecting — raw_position is in OS window coords which may differ
          // from the design resolution when WINDOW_X/Y != VIEWPORT_X/Y.
          float mouse_x = event->raw_position.x * float(TESettings::VIEWPORT_X) / TESettings::WINDOW_X;
          float mouse_y = event->raw_position.y * float(TESettings::VIEWPORT_Y) / TESettings::WINDOW_Y;
          float win_x = mouse_x;
          float win_y = viewport.w - mouse_y; // OpenGL's y is from bottom
          glm::vec3 cam_pos = glm::vec3(glm::inverse(view)[3]);

          // Unproject to world space (near and far plane)
          glm::vec3 near_point = glm::unProject(glm::vec3(win_x, win_y, 0.0f), view, proj, viewport);
          glm::vec3 far_point  = glm::unProject(glm::vec3(win_x, win_y, 1.0f), view, proj, viewport);

          // Ray from camera position to far_point
          glm::vec3 ray_dir = glm::normalize(far_point - near_point);

          if(!camera->perspective) {
            ray_dir = -glm::normalize(camera->get_forward());
          }

          glm::vec3 box_min, box_max;
          picking_aabb(entity, transform, box_min, box_max);

          return intersect_ray_aabb(near_point, ray_dir, box_min, box_max);
        }
    }
    return false;
    }

    inline bool hovered_over(MouseMoveEvent *event, Entity entity,
                             float zoom = 1.0f) {
      auto transform = TE::get_component<Transform>(entity);
      auto animation = TE::get_component<Animation>(entity);
      if(animation != nullptr && animation->playing) {
        transform = animation->root->transform;
      }
      if (transform != nullptr) {
        float w = transform->width / zoom;
        float h = transform->height / zoom;
        glm::vec2 pos =
            glm::vec2(transform->get_centered_position_from_camera().x / zoom,
                      transform->get_centered_position_from_camera().y / zoom);

        return ((event->screen_position.x > pos.x - w &&
                 event->screen_position.x < pos.x + w) &&
                (event->screen_position.y > pos.y - h &&
                 event->screen_position.y < pos.y + h));
      }
      return false;
    }

    inline bool hovered_over_perspective(MouseMoveEvent *event, Entity entity,
                                         float zoom = 1.0f) {
      auto transform = TE::get_component<Transform>(entity);
      // Match clicked_on_perspective: while an animation is playing it is the
      // animation's root copy that gets rendered, so pick against that one or
      // hover and click disagree mid-animation.
      auto animation = TE::get_component<Animation>(entity);
      if (animation != nullptr && animation->playing) {
        transform = animation->root->transform;
      }
      if (transform != nullptr) {
        // Get camera matrices and viewport
        auto camera = transform->camera;
        if (camera != nullptr) {
          glm::mat4 view = camera->get_view_matrix();
          glm::mat4 proj = camera->get_projection_matrix(TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y, TESettings::ZOOM);
          // glm::mat4 proj = camera->get_projection_matrix();
          glm::vec4 viewport =
              glm::vec4(0, 0, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);

          // Scale raw window pixels to design-resolution (viewport) pixels.
          float mouse_x = event->raw_position.x * float(TESettings::VIEWPORT_X) / TESettings::WINDOW_X;
          float mouse_y = event->raw_position.y * float(TESettings::VIEWPORT_Y) / TESettings::WINDOW_Y;
          float win_x = mouse_x;
          float win_y = viewport.w - mouse_y; // OpenGL's y is from bottom

          // Unproject to world space (near and far plane)
          glm::vec3 near_point = glm::unProject(glm::vec3(win_x, win_y, 0.0f),
                                                view, proj, viewport);
          glm::vec3 far_point = glm::unProject(glm::vec3(win_x, win_y, 1.0f),
                                               view, proj, viewport);

          // Ray from camera position to far_point
          glm::vec3 ray_dir = glm::normalize(far_point - near_point);

          if(!camera->perspective) {
            ray_dir = -glm::normalize(camera->get_forward());
          }

          glm::vec3 box_min, box_max;
          picking_aabb(entity, transform, box_min, box_max);

          return intersect_ray_aabb(near_point, ray_dir, box_min, box_max);
        }
      }
      return false;
    }
};
