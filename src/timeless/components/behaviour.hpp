#pragma once
#include "component.hpp"
#include "timeless/entity.hpp"
#include <deque>
#include <functional>
#include <memory>
#include <vector>

class Instruction
{
private:
    std::function<void(Entity entity, bool reverse, float speed)> instruct_func;

public:
    Instruction(std::function<void(Entity entity, bool reverse, float speed)> instruct_func)
        : instruct_func(instruct_func)
    {
    }

    Instruction()
    {
    }

    void run(Entity entity, bool reverse = false, float speed = 1.0)
    {
        // run this instruction
        instruct_func(entity, reverse, speed);
    }
};

const Instruction idle = Instruction([](Entity entity, bool reverse, float speed) {});

/** The behaviour component holds a list of commands
 * that an NPC will carry out. Ticks/time-units are defined,
 * and when a new event is needed, it will be fetched from the
 * list in here.
 */
class Behaviour : public Component
{
private:
    int max = 12 * 12;

public:
    std::vector<Instruction> next_instructions;
    std::shared_ptr<Instruction> callback;

    Behaviour(std::vector<Instruction> f_instructions)
        : next_instructions(f_instructions)
    {
    }

    /** fill instructions with idle to ensure time max is reached*/
    void fill_instructions_with_idle()
    {
        if (next_instructions.size() < max)
        {
            for (int i = 0; i < max - next_instructions.size(); i++)
            {
                next_instructions.push_back(idle);
            }
        }
    }

    void add_instructions(std::vector<Instruction> f_instructions)
    {
        next_instructions.insert(next_instructions.end(), f_instructions.begin(), f_instructions.end());
    }

    /** copy set instructions n times so it fits in max timeline*/
    void loop_instructions()
    {
        int n = floor(max / next_instructions.size());

        for (int i = 0; i < n; i++)
        {
            next_instructions.insert(next_instructions.end(), next_instructions.begin(), next_instructions.end());
        }

        next_instructions.resize(max);
    }

    std::shared_ptr<Instruction> next(int index)
    {
        if (index > next_instructions.size() - 1)
        {
            return nullptr;
        }
        std::shared_ptr<Instruction> instr = std::make_shared<Instruction>(next_instructions.at(index));
        return instr;
    }
};
