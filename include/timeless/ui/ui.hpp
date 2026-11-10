#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "timeless/core.hpp"
#include "timeless/entity.hpp"

class UIComponent {
private:
  std::vector<Entity> entities;
  bool removed = false;

public:
  Entity main_ent;
  Entity bg_ent;
  bool hovering = false;

  // Zeroed: glm vectors aren't, and the default constructor doesn't set these
  // (only update_position() does), so reading them before that got whatever
  // was in memory.
  glm::vec3 position{0.0f};
  glm::vec2 anchor{0.0f};
  glm::vec2 margin{0.0f};

  UIComponent(glm::vec2 anchor, glm::vec2 margin) : anchor(anchor), margin(margin) {
    main_ent = create_entity("uicomponent_main"); 
    bg_ent = create_entity("uicomponent_bg");

    add_entity(main_ent);
    add_entity(bg_ent);
  }

  UIComponent() {
    main_ent = create_entity("uicomponent_main"); 
    bg_ent = create_entity("uicomponent_bg");

    add_entity(main_ent);
    add_entity(bg_ent);
  }

  void update_position();

  void add_entity(Entity entity) { 
    entities.push_back(entity); 
  }

  virtual void create_animation();

  virtual void hover_animation();

  virtual void exit_animation();

  bool is_removed() const { return removed; }

  virtual void remove();

  virtual void add_close_event_listener(Entity entity);

  virtual void add_hover_event_listener();

};
