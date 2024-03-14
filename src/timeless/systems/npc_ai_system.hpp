#pragma once
#include "../timer.hpp"
#include "system.hpp"

class NpcAiSystem : public System
{
private:
    Timer timer = Timer(1.0f);
    bool forward = true;
    int start_hour = 7;
    int max_time = 200;

    Entity index_text;

public:
    float speed = 1.0f;
    bool running = true;
    bool viewing_ui= false; // separate for when viewing UI etc

    int main_index = -1;

    void attach_text_component(ComponentManager &cm, Entity text)
    {
        index_text = text;
        update_text(cm);
    }

    void change_timer(float new_time)
    {
        timer = Timer(new_time);
        speed = new_time;
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
    }
    void resume(ComponentManager &cm)
    {
        running = true;
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
					auto instr = behaviour->next(main_index);
					if (instr != nullptr)
					{
                        bool reverse = !forward;
						instr->run(entity, reverse);
					}
                }
				update_text(cm);
            }
        }
    }
};
