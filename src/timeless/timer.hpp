#pragma once
#include <GLFW/glfw3.h>

class Timer
{
private:
    double start;
    double secondsToWait;

public:
    Timer()
    {
        secondsToWait = 1.0;
        start = glfwGetTime();
    }
    Timer(double secondsToWait)
        : secondsToWait(secondsToWait)
    {
        start = glfwGetTime();
    }
    void startTimer()
    {
        start = glfwGetTime();
    }
    bool pollTime()
    {
        double current = glfwGetTime();
        if (current - start >= secondsToWait)
        {
            start = glfwGetTime();
            return true;
        }
        return false;
    }
};