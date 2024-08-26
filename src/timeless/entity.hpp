#pragma once
#include <stdint.h>

using Entity = uint32_t;
const Entity MAX_ENTITIES = 9000;

Entity create_entity()
{
    static Entity entities = 0;
    ++entities;
    return entities;
}