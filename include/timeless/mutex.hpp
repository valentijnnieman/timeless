#pragma once
#ifdef __EMSCRIPTEN__
#include <mutex>
#include <shared_mutex>
// A mutex that does absolutely nothing
struct NullMutex {
    void lock() {}
    void unlock() {}
    bool try_lock() { return true; }
};
using Mutex = NullMutex;
using SharedMutex = NullMutex;
#else
#include <mutex>
#include <shared_mutex>
using Mutex = std::mutex;
using SharedMutex = std::shared_mutex;
#endif

