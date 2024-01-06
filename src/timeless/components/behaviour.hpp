#pragma once
#include <deque>

class Instruction
{
private:
    std::function<void(Entity entity)> instruct_func;

public:
    Instruction(std::function<void(Entity entity)> instruct_func)
        : instruct_func(instruct_func)
    {
    }

    void run(Entity entity)
    {
        // run this instruction
        instruct_func(entity);
    }
};

const Instruction idle = Instruction([](Entity entity) {});

/** The behaviour component holds a list of commands
 * that an NPC will carry out. Ticks/time-units are defined,
 * and when a new event is needed, it will be fetched from the
 * deque in here.
 */
class Behaviour
{
private:
    int index = 0;
    int max = 12 * 12;

    std::vector<std::shared_ptr<Instruction>> next_instructions;
    std::vector<std::shared_ptr<Instruction>> prev_instructions;

public:
    Behaviour(std::vector<std::shared_ptr<Instruction>> f_instructions, std::vector<std::shared_ptr<Instruction>> b_instructions)
        : next_instructions(f_instructions), prev_instructions(b_instructions)
    {
        if (next_instructions.size() != prev_instructions.size())
        {
            throw std::invalid_argument("Both next and previous instructions must be of the same size");
        }
    }

    /** fill instructions with idle to ensure time max is reached*/
    void fill_instructions_with_idle()
    {
        if (next_instructions.size() < max && prev_instructions.size() < max)
        {
            for (int i = 0; i < max - next_instructions.size(); i++)
            {
                next_instructions.push_back(std::make_shared<Instruction>(idle));
            }
            for (int i = 0; i < max - prev_instructions.size(); i++)
            {
                prev_instructions.push_back(std::make_shared<Instruction>(idle));
            }
        }
    }

    void add_instructions(std::vector<std::shared_ptr<Instruction>> f_instructions, std::vector<std::shared_ptr<Instruction>> b_instructions)
    {
        next_instructions.insert(next_instructions.end(), f_instructions.begin(), f_instructions.end());
        prev_instructions.insert(prev_instructions.end(), b_instructions.begin(), b_instructions.end());

        fill_instructions_with_idle();
    }

    /** copy set instructions n times so it fits in max timeline*/
    void loop_instructions()
    {
        int n = floor(max / next_instructions.size());

        for (int i = 0; i < n; i++)
        {
            next_instructions.insert(next_instructions.end(), next_instructions.begin(), next_instructions.end());

            prev_instructions.insert(prev_instructions.end(), prev_instructions.begin(), prev_instructions.end());
        }

        next_instructions.resize(max);
        prev_instructions.resize(max);
    }

    void advance_index()
    {
        if (index < next_instructions.size() - 1)
        {
            index++;
        }
    }
    void decrease_index()
    {
        if (index > 0)
        {
            index--;
        }
    }

    std::shared_ptr<Instruction> next()
    {
        if (index >= next_instructions.size() - 1)
        {
            return nullptr;
        }
        std::shared_ptr<Instruction> instr = next_instructions.at(index);
        return instr;
    }
    std::shared_ptr<Instruction> prev()
    {
        if (index <= 0)
        {
            return nullptr;
        }
        std::shared_ptr<Instruction> instr = prev_instructions.at(index);
        return instr;
    }
};