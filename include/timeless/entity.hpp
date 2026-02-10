#pragma once
#include <stdint.h>
#include <unordered_set>
#include <vector>

using Entity = uint32_t;
const Entity MAX_ENTITIES = UINT32_MAX;

static Entity entities = 0;
static bool out_of_ids = false;
static std::unordered_set<Entity> active_entities;

inline Entity create_entity(const std::string& debug_name = "") {
  Entity id;
  if (entities >= MAX_ENTITIES) {
      if(out_of_ids) {
          throw std::runtime_error("ERROR: Cannot create more entities, maximum limit reached.");
      }
      std::cout << "ERROR: Maximum number of entities reached!" << std::endl;
      entities = 0;
      out_of_ids = true;
      return create_entity();
  }
  id = ++entities;
  if (active_entities.contains(id)) {
      if(debug_name != "")
        std::cout << "Failed Creating entity: " << debug_name << " " << id << std::endl;
      std::cout << "ERROR: Entity already exists: " << id << std::endl;
      return create_entity();
  }
  active_entities.insert(id);
  return id;
}

inline void destroy_entity(Entity id) {
    active_entities.erase(id);
    // free_ids.push_back(id);
}
