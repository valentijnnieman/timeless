#pragma once
#include <deque>

class Instruction
{
private:
    std::function<void(Entity entity, bool reverse)> instruct_func;

public:
    Instruction(std::function<void(Entity entity, bool reverse)> instruct_func)
        : instruct_func(instruct_func)
    {
    }

    void run(Entity entity, bool reverse = false)
    {
        // run this instruction
        instruct_func(entity, reverse);
    }
};

const Instruction idle = Instruction([](Entity entity, bool reverse) {});

/** The behaviour component holds a list of commands
 * that an NPC will carry out. Ticks/time-units are defined,
 * and when a new event is needed, it will be fetched from the
 * list in here.
 */
class Behaviour
{
private:
    int max = 12 * 12;

public:
    std::vector<std::shared_ptr<Instruction>> next_instructions;

    Behaviour(std::vector<std::shared_ptr<Instruction>> f_instructions)
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
                next_instructions.push_back(std::make_shared<Instruction>(idle));
            }
        }
    }

    void add_instructions(std::vector<std::shared_ptr<Instruction>> f_instructions)
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
        if (index >= next_instructions.size() - 1)
        {
            return nullptr;
        }
        std::shared_ptr<Instruction> instr = next_instructions.at(index);
        return instr;
    }
};