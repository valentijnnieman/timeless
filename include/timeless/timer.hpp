#pragma once
#include "timeless/input.hpp"
#include <functional>
#include <vector>

class Timer
{
private:
    double start;
    double secondsToWait;

public:
    Timer()
    {
        secondsToWait = 1.0;
        start = TE::now_seconds();
    }
    Timer(double secondsToWait)
        : secondsToWait(secondsToWait)
    {
        start = TE::now_seconds();
    }
    void startTimer()
    {
        start = TE::now_seconds();
    }
    bool pollTime()
    {
        double current = TE::now_seconds();
        if (current - start >= secondsToWait)
        {
            start = TE::now_seconds();
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

    void update();
};
