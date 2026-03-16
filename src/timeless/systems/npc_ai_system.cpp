#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif
#include "timeless/systems/npc_ai_system.hpp"

void NpcAiSystem::attach_text_component(ComponentManager &cm, Entity text) {
    index_text = text;
    update_text(cm);
}

void NpcAiSystem::change_timer(float new_time) {
    timer = Timer(new_time);
    speed = new_time;
}

void NpcAiSystem::play_forward(ComponentManager &cm) {
    if (!forward || !running) {
        forward = true;
        running = true;
    }
}

void NpcAiSystem::play_backward(ComponentManager &cm) {
    if (forward || !running) {
        forward = false;
        running = true;
    }
}

void NpcAiSystem::pause(ComponentManager &cm) { running = false; }
void NpcAiSystem::resume(ComponentManager &cm) { running = true; }

std::string NpcAiSystem::index_to_time() {
    int minutes  = main_index * 5;
    int remainder = minutes % 60;
    int hours    = (minutes - remainder) / 60;
    hours += start_hour;
    if (hours < 10)
        return "0" + std::to_string(hours) + "." + std::to_string(remainder);
    return std::to_string(hours) + "." + std::to_string(remainder);
}

void NpcAiSystem::update_text(ComponentManager &cm) {
    auto text = cm.get_component<Text>(index_text);
    if (text != nullptr) {
        auto time = index_to_time();
        text->text    = time + " tick " + std::to_string(main_index);
        text->printed = time + " tick " + std::to_string(main_index);
    }
}

void NpcAiSystem::update(ComponentManager &cm) {
    if (timer.pollTime()) {
        if (running) {
            bool reverse = !forward;
            for (auto &entity : registered_entities) {
                auto behaviour = cm.get_component<Behaviour>(entity);
                auto instr     = behaviour->next(main_index);
                if (instr != nullptr)
                    instr->run(entity, reverse, speed);
            }
            update_text(cm);
            update_text(cm);
            if (!forward) {
                if (main_index > 0) main_index--;
            } else if (forward) {
                if (main_index < max_time) main_index++;
            }
        }
    }
}
