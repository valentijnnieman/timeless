#pragma once
#include <stdint.h>
#include <vector>

using Entity = uint32_t;
const Entity MAX_ENTITIES = 9000;

static Entity entities = 0;
static std::vector<Entity> free_ids;

Entity create_entity() {
    if (!free_ids.empty()) {
        Entity id = free_ids.back();
        free_ids.pop_back();
        return id;
    }
    if (entities >= MAX_ENTITIES)
        return 0; // or UINT32_MAX as invalid
    return ++entities;
}

void destroy_entity(Entity id) {
    free_ids.push_back(id);
}
