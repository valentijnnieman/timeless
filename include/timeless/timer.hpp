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

class TimerManager {
public:
    struct TimerEntry {
        Timer timer;
        std::function<void()> callback;
    };
    std::vector<TimerEntry> timers;

    void addTimer(double seconds, std::function<void()> callback) {
        timers.push_back({Timer(seconds), callback});
    }

    void clearTimers() {
        timers.clear();
    }

    void update() {
        for (auto it = timers.begin(); it != timers.end(); ) {
            if (it->timer.pollTime()) {
                it->callback();
                it = timers.erase(it);
            } else {
                ++it;
            }
        }
    }
};
