#include "timeless/systems/system.hpp"
#include <algorithm>

void System::register_entity(Entity entity) {
  registered_entities.push_back(entity);
}

void System::remove_entity(Entity entity) {
  if (!registered_entities.empty()) {
    auto found =
        std::find_if(registered_entities.begin(), registered_entities.end(),
                     [&](auto &e) { return e == entity; });
    if (found != registered_entities.end()) {
      registered_entities.erase(found);
    }
  }
}

void System::clear(ComponentManager &cm) {
  if (!registered_entities.empty()) {
    for (auto ent : registered_entities) {
      cm.remove_entity(ent);
    }
    registered_entities.clear();
  }
}

int System::get_registered_entity_count() {
  return registered_entities.size();
}

void System::purge() {
  registered_entities.clear();
}
