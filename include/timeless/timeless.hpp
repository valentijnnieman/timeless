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
#include "timeless/systems/mouse_input_system.hpp"
#include "timeless/systems/movement_system.hpp"
#include "timeless/systems/keyboard_input_system.hpp"
#include "timeless/systems/npc_ai_system.hpp"
#include "timeless/systems/inventory_system.hpp"
#include "timeless/systems/event_system.hpp"
#include "timeless/systems/animation_system.hpp"
#include "timeless/systems/camera_system.hpp"
#include "timeless/systems/sound_system.hpp"
#include "timeless/algorithm/graph.hpp"
#include "timeless/systems/system.hpp"

namespace TE
{
    inline std::map<std::string, std::shared_ptr<System>> systems;

    inline std::shared_ptr<ComponentManager> cm;
    inline std::shared_ptr<MouseInputSystem> mis;
    inline std::shared_ptr<WindowManager> wm;
    inline std::shared_ptr<Grid> grid;

    inline void init()
    {
        cm = std::make_shared<ComponentManager>();
        mis = std::make_shared<MouseInputSystem>();
        wm = std::make_shared<WindowManager>(cm, mis);

        grid = std::make_shared<Grid>(Grid());
    }

    inline void cleanup()
    {
        std::cout << "TE Cleanup called" << std::endl;

        for (const auto& [key, system] : systems)
        {
            system->purge();
        }

        // mis.reset();

        cm->cleanup();
        cm.reset();

        grid.reset();

        wm->cleanup();
        wm.reset();

        std::cout << "TE Cleanup finished" << std::endl;
        std::cout << "Terminating GLFW..." << std::endl;
        glfwTerminate();
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

    template <typename T>
    inline std::shared_ptr<T> get_system(const std::string &key) {
      return std::dynamic_pointer_cast<T>(systems[key]);
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

    inline void remove_entity(Entity entity)
    {
        cm->remove_entity(entity);
        mis->remove_entity(entity);
        mis->remove_move_entity(entity);
        for (const auto& [key, system] : systems)
        {
            system->remove_entity(entity);
        }
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
        grid->register_entity(entity);
    }
    template<> inline void add_component<Node>(Entity entity, std::shared_ptr<Node> node)
    {
        cm->add_component(entity, node);
        grid->register_entity(entity);
    }

    /** TODO: return shared_ptr instead of ref to unique_ptr? refs to smart pointers
     * defeat the purpose of smart pointers - although it's not likely our system
     * and manager pointers go out of scope before the program finishes, probably
     * better to do something else here.
     */
    inline std::shared_ptr<ComponentManager> get_component_manager()
    {
        return cm;
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

    inline void loop(std::function<void(GLFWwindow *window, ComponentManager &cm, WindowManager &wm)> loop_func)
    {
        loop_func(wm->window, *cm, *wm);
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
          glm::vec3 rotated_mouse = inv_rot * mouse_pos;

          return ((rotated_mouse.x > pos.x - w && 
                  rotated_mouse.x < pos.x + w) &&
                  (rotated_mouse.y > pos.y - h && 
                  rotated_mouse.y < pos.y + h)
            );
      }
      return false;
    }
    //
    inline bool intersect_ray_aabb(const glm::vec3 &origin, const glm::vec3 &dir,
                                  const glm::vec3 &aabb_min, const glm::vec3 &aabb_max) {
      float tmin = (aabb_min.x - origin.x) / dir.x;
      float tmax = (aabb_max.x - origin.x) / dir.x;
      if (tmin > tmax) std::swap(tmin, tmax);

      float tymin = (aabb_min.y - origin.y) / dir.y;
      float tymax = (aabb_max.y - origin.y) / dir.y;
      if (tymin > tymax) std::swap(tymin, tymax);

      if ((tmin > tymax) || (tymin > tmax))
        return false;

      if (tymin > tmin)
        tmin = tymin;
      if (tymax < tmax)
        tmax = tymax;

      float tzmin = (aabb_min.z - origin.z) / dir.z;
      float tzmax = (aabb_max.z - origin.z) / dir.z;
      if (tzmin > tzmax) std::swap(tzmin, tzmax);

      if ((tmin > tzmax) || (tzmin > tmax))
        return false;

      return true;
    }

    inline bool clicked_on_perspective(MouseEvent* event, Entity entity, float zoom = 1.0f)
    {
    auto transform = TE::get_component<Transform>(entity);
    if(transform != nullptr) {
        // Get camera matrices and viewport
        auto camera = transform->camera;
        glm::mat4 view = camera->get_view_matrix();
        glm::mat4 proj = camera->get_projection_matrix(TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y, TESettings::ZOOM);
        // glm::mat4 proj = camera->get_projection_matrix();

        // glm::vec4 viewport = camera->get_viewport(); // (x, y, width, height)
        glm::vec4 viewport = glm::vec4(0, 0, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);

        // Mouse position to NDC
        float mouse_x = event->raw_position.x;
        float mouse_y = event->raw_position.y;
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

        // Entity bounding box (centered position, width, height)
        glm::vec3 box_center = transform->get_position();
        float w = transform->width;
        float h = transform->height;
        glm::vec3 box_min = box_center - glm::vec3(w, h, 0.0f);
        glm::vec3 box_max = box_center + glm::vec3(w, h, 0.0f);

        return intersect_ray_aabb(near_point, ray_dir, box_min, box_max);
    }
    return false;
    }

    inline bool hovered_over(MouseMoveEvent *event, Entity entity,
                             float zoom = 1.0f) {
      auto transform = TE::get_component<Transform>(entity);
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
      if (transform != nullptr) {
        // Get camera matrices and viewport
        auto camera = transform->camera;
        if (camera != nullptr) {
          glm::mat4 view = camera->get_view_matrix();
          glm::mat4 proj = camera->get_projection_matrix(TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y, TESettings::ZOOM);
          // glm::mat4 proj = camera->get_projection_matrix();
          glm::vec4 viewport =
              glm::vec4(0, 0, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);

          // Mouse position to NDC
          float mouse_x = event->raw_position.x;
          float mouse_y = event->raw_position.y;
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

          // Entity bounding box (centered position, width, height)
          glm::vec3 box_center = transform->get_position();
          float w = transform->width;
          float h = transform->height;
          glm::vec3 box_min = box_center - glm::vec3(w, h, 0.0f);
          glm::vec3 box_max = box_center + glm::vec3(w, h, 0.0f);

          return intersect_ray_aabb(near_point, ray_dir, box_min, box_max);
        }
      }
      return false;
    }
};
