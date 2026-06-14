#pragma once
#include "timeless/input.hpp"
#include <functional>

class Timer
{
private:
    double start;
    double secondsToWait;

public:
    Timer()
    {
        secondsToWait = 1.0;
        start = te::now_seconds();
    }
    Timer(double secondsToWait)
        : secondsToWait(secondsToWait)
    {
        start = te::now_seconds();
    }
    void startTimer()
    {
        start = te::now_seconds();
    }
    bool pollTime()
    {
        double current = te::now_seconds();
        if (current - start >= secondsToWait)
        {
            start = te::now_seconds();
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
        // Collect fired callbacks first to avoid iterator invalidation
        // when a callback itself calls addTimer().
        std::vector<std::function<void()>> fired;
        for (auto it = timers.begin(); it != timers.end(); ) {
            if (it->timer.pollTime()) {
                fired.push_back(std::move(it->callback));
                it = timers.erase(it);
            } else {
                ++it;
            }
        }
        for (auto& cb : fired) {
            cb();
        }
    }
};
