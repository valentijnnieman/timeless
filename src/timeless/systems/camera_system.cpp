#include "timeless/systems/camera_system.hpp"

void CameraSystem::update(ComponentManager &cm, float delta_time) {
    for (auto entity : registered_entities) {
        auto camera = cm.get_component<Camera>(entity);
        camera->update(delta_time);
    }
}
