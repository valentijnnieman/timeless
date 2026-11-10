#include "timeless/components/behaviour.hpp"
#include <cmath>

void Behaviour::fill_instructions_with_idle() {
    if (next_instructions.size() < max)
    {
        for (int i = 0; i < max - next_instructions.size(); i++)
        {
            next_instructions.push_back(idle);
        }
    }
}

void Behaviour::loop_instructions() {
    int n = floor(max / next_instructions.size());
    const std::vector<Instruction> original = next_instructions;

    for (int i = 0; i < n; i++)
    {
        next_instructions.insert(next_instructions.end(), original.begin(), original.end());
    }

    next_instructions.resize(max);
}
