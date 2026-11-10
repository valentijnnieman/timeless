#include "timeless/core.hpp"

namespace TE
{

void init() {
    cm = std::make_shared<ComponentManager>();
    tm = std::make_shared<TimerManager>();
    mis = std::make_shared<MouseInputSystem>();
    wm = std::make_shared<WindowManager>(cm, mis);

    grid = std::make_shared<Grid>(Grid());
}

void cleanup() {
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

void remove_system(const std::string& key) {
  systems[key]->purge();
  systems.erase(key);
}

void print_systems() {
  for (const auto& [key, system] : systems)
  {
      std::cout << "System Key: " << key << std::endl;
  }
}

void remove_entity(Entity entity, bool destroy) {
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

bool clicked_on(MouseEvent* event, Entity entity, float zoom) {
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

bool intersect_ray_aabb(const glm::vec3 &origin, const glm::vec3 &dir,
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

bool posed_local_aabb(Entity entity, const std::shared_ptr<Model> &model,
                      glm::vec3 &out_min, glm::vec3 &out_max) {
  auto anim = TE::get_component<SkeletalAnimation>(entity);
  if (anim == nullptr || model->bone_aabb_min.empty())
    return false;
  anim->ensure_pose();

  // Accumulate separately: on failure the caller keeps its bind box.
  glm::vec3 posed_min(std::numeric_limits<float>::max());
  glm::vec3 posed_max(std::numeric_limits<float>::lowest());
  auto add_box = [&](const glm::mat4 &m, const glm::vec3 &lo, const glm::vec3 &hi) {
    for (int c = 0; c < 8; ++c) {
      glm::vec3 corner((c & 1) ? hi.x : lo.x, (c & 2) ? hi.y : lo.y,
                       (c & 4) ? hi.z : lo.z);
      glm::vec3 p = glm::vec3(m * glm::vec4(corner, 1.0f));
      posed_min = glm::min(posed_min, p);
      posed_max = glm::max(posed_max, p);
    }
  };
  bool any = false;
  for (size_t i = 0; i < model->bone_aabb_min.size(); ++i) {
    const glm::vec3 &lo = model->bone_aabb_min[i];
    const glm::vec3 &hi = model->bone_aabb_max[i];
    if (lo.x > hi.x) continue; // bone drives no vertices
    // Bones past poseMatrices are uploaded as identity (see
    // RenderingSystem::upload_bone_matrices).
    glm::mat4 pose = i < anim->poseMatrices.size() ? anim->poseMatrices[i]
                                                   : glm::mat4(1.0f);
    add_box(pose, lo, hi);
    any = true;
  }
  if (model->has_unskinned_aabb) {
    add_box(glm::mat4(1.0f), model->unskinned_aabb_min, model->unskinned_aabb_max);
    any = true;
  }
  if (!any)
    return false;
  out_min = posed_min;
  out_max = posed_max;
  return true;
}

void picking_aabb(Entity entity, const std::shared_ptr<Transform>& transform,
                  glm::vec3 &out_min, glm::vec3 &out_max) {
  auto model = TE::get_component<Model>(entity);
  if (model != nullptr && model->has_local_aabb) {
    glm::vec3 lo = model->local_aabb_min;
    glm::vec3 hi = model->local_aabb_max;
    posed_local_aabb(entity, model, lo, hi);

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

bool picks_upright(Entity entity, const std::shared_ptr<Transform> &transform) {
  if (!transform->pick_upright) return false;
  auto model = TE::get_component<Model>(entity);
  return model != nullptr && model->has_local_aabb;
}

bool hit_upright(float win_x, float win_y, Entity entity,
                 const std::shared_ptr<Transform> &transform,
                 const glm::mat4 &view, const glm::mat4 &proj,
                 const glm::vec4 &viewport) {
  glm::vec3 box_min, box_max;
  picking_aabb(entity, transform, box_min, box_max);

  glm::vec3 center = (box_min + box_max) * 0.5f;
  float radius = 0.5f * std::min(box_max.x - box_min.x, box_max.y - box_min.y);
  // Camera right in world space, to measure the figure's on-screen width.
  glm::vec3 right = glm::normalize(glm::vec3(glm::inverse(view)[0]));

  glm::vec3 feet_world(center.x, center.y, box_min.z);
  glm::vec3 feet = glm::project(feet_world, view, proj, viewport);
  glm::vec3 head = glm::project(glm::vec3(center.x, center.y, box_max.z), view, proj, viewport);
  glm::vec3 side = glm::project(feet_world + right * radius, view, proj, viewport);
  float half_w = std::abs(side.x - feet.x);

  // win_x/win_y and glm::project share the same bottom-left origin.
  return win_x >= std::min(feet.x, head.x) - half_w &&
         win_x <= std::max(feet.x, head.x) + half_w &&
         win_y >= std::min(feet.y, head.y) &&
         win_y <= std::max(feet.y, head.y);
}

bool clicked_on_perspective(MouseEvent* event, Entity entity, float zoom) {
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

      if (picks_upright(entity, transform))
        return hit_upright(win_x, win_y, entity, transform, view, proj, viewport);

      glm::vec3 box_min, box_max;
      picking_aabb(entity, transform, box_min, box_max);

      return intersect_ray_aabb(near_point, ray_dir, box_min, box_max);
    }
}
return false;
}

bool hovered_over(MouseMoveEvent *event, Entity entity, float zoom) {
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

bool hovered_over_perspective(MouseMoveEvent *event, Entity entity,
                              float zoom) {
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

      if (picks_upright(entity, transform))
        return hit_upright(win_x, win_y, entity, transform, view, proj, viewport);

      glm::vec3 box_min, box_max;
      picking_aabb(entity, transform, box_min, box_max);

      return intersect_ray_aabb(near_point, ray_dir, box_min, box_max);
    }
  }
  return false;
}

} // namespace TE
