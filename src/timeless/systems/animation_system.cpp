#include "timeless/systems/animation_system.hpp"

void AnimationSystem::update(ComponentManager &cm, float delta_time) {
    for (auto entity : registered_entities) {
        auto animation = cm.get_component<Animation>(entity);
        if (animation != nullptr) {
            if (animation->playing) {
                animation->update(delta_time);
            }
        }
        auto skeletal_animation = cm.get_component<SkeletalAnimation>(entity);
        if (skeletal_animation != nullptr) {
            if (skeletal_animation->playing) {
                skeletal_animation->update(delta_time);
            }
        }
    }
}
