#pragma once
#include "../timer.hpp"

class NpcAiSystem
{
private:
    Timer timer = Timer(0.5f);
    bool forward = true;
    int start_hour = 9;

    Entity index_text;

public:
    std::vector<Entity> registered_entities;
    bool running = true;

    int main_index = 0;

    void attach_text_component(ComponentManager &cm, Entity text)
    {
        index_text = text;
        update_text(cm);
    }

    void register_entity(Entity entity)
    {
        registered_entities.push_back(entity);
    }
    void remove_entity(Entity entity)
    {
        if (!registered_entities.empty())
        {
            auto found = std::find_if(registered_entities.begin(), registered_entities.end(), [&](auto &e)
                                      { return e == entity; });
            if (found != registered_entities.end())
            {
                registered_entities.erase(found);
            }
        }
    }

    void play_forward(ComponentManager &cm)
    {
        if (!forward)
        {
            for (auto &entity : registered_entities)
            {
                auto behaviour = cm.get_behaviour(entity);
                behaviour->advance_index();
            }
            forward = true;
            running = true;
        }
    }
    void play_backward(ComponentManager &cm)
    {
        if (forward)
        {
            for (auto &entity : registered_entities)
            {
                auto behaviour = cm.get_behaviour(entity);
                behaviour->decrease_index();
            }
            forward = false;
            running = true;
        }
    }

    void pause(ComponentManager &cm)
    {
        running = false;
        for (auto &entity : registered_entities)
        {
            auto sprite = cm.get_sprite(entity);
            sprite->animating = false;
        }
    }
    void resume(ComponentManager &cm)
    {
        running = true;
        for (auto &entity : registered_entities)
        {
            auto sprite = cm.get_sprite(entity);
            sprite->animating = true;
        }
    }

    std::string index_to_time()
    {
        int minutes = main_index * 5;
        int remainder = minutes % 60;
        int hours = (minutes - remainder) / 60;
        hours += start_hour;

        if (hours < 10)
        {
            return "0" + std::to_string(hours) + "." + std::to_string(remainder);
        }
        return std::to_string(hours) + "." + std::to_string(remainder);
    }

    void update_text(ComponentManager &cm)
    {
        auto text = cm.get_text(index_text);
        if (text != nullptr)
        {
            auto time = index_to_time();
            text->text = time;
            text->printed = time;
        }
    }

    void update(ComponentManager &cm)
    {
        if (timer.pollTime())
        {
            if (running)
            {
                bool did_update = false;
                for (auto &entity : registered_entities)
                {
                    auto behaviour = cm.get_behaviour(entity);
                    auto sprite = cm.get_sprite(entity);
                    sprite->animating = true;
                    if (forward)
                    {
                        auto instr = behaviour->next();
                        if (instr != nullptr)
                        {
                            instr->run(entity);
                            behaviour->advance_index();
                            did_update = true;
                        }
                    }
                    else
                    {
                        auto instr = behaviour->prev();
                        if (instr != nullptr)
                        {
                            instr->run(entity);
                            behaviour->decrease_index();
                            did_update = true;
                        }
                    }
                }

                if (did_update)
                {
                    // for (auto &entity : registered_entities)
                    // {
                    //     auto sprite = cm.get_sprite(entity);
                    //     // sprite->animating = false;
                    // }
                    if (forward)
                    {
                        main_index++;
                        update_text(cm);
                    }
                    else
                    {
                        main_index--;
                        update_text(cm);
                    }
                }
                else
                {
                    for (auto &entity : registered_entities)
                    {
                        auto sprite = cm.get_sprite(entity);
                        sprite->animating = false;
                    }
                }
            }
        }
    }
};
