#pragma once
#include "../timer.hpp"

class NpcAiSystem
{
private:
    Timer timer = Timer(0.8f);
    bool forward = true;
    int start_hour = 8;
    int max_time = 400;

    Entity index_text;

public:
    std::vector<Entity> registered_entities;
    bool running = true;
    bool viewing_ui= false; // separate for when viewing UI etc

    int main_index = -1;

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
            forward = true;
            running = true;
        }
    }
    void play_backward(ComponentManager &cm)
    {
        if (forward)
        {
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
            //sprite->animating = false;
        }
    }
    void resume(ComponentManager &cm)
    {
        running = true;
        for (auto &entity : registered_entities)
        {
            auto sprite = cm.get_sprite(entity);
            //sprite->animating = true;
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
            text->text = time + " tick " + std::to_string(main_index);
            text->printed = time + " tick " + std::to_string(main_index);
        }
    }

    void update(ComponentManager &cm)
    {
        if (timer.pollTime())
        {
            if (running)
            {
                if(!forward)
				{
                    if(main_index > 0)
						main_index--;
				}
				else if (forward)
				{
                    if(main_index < max_time)
						main_index++;
				}
                for (auto &entity : registered_entities)
                {
                    auto behaviour = cm.get_behaviour(entity);
                    auto sprite = cm.get_sprite(entity);
					auto instr = behaviour->next(main_index);
                    //sprite->animating = false;
					if (instr != nullptr)
					{
                        bool reverse = !forward;
						instr->run(entity, reverse);
						//sprite->animating = true;
					}
                }
				update_text(cm);
            }
        }
    }
};
