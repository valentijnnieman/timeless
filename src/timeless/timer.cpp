#include "timeless/timer.hpp"
#include <utility>

void TimerManager::update() {
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
