#pragma once
#ifdef __EMSCRIPTEN__
#include <mutex>
#include <shared_mutex>
using Mutex = std::mutex;
using SharedMutex = std::shared_mutex;
// #include <mutex>
// #include <shared_mutex>
// // A mutex that does absolutely nothing
// struct NullMutex {
//     void lock() {}
//     void lock_shared() {}
//     void unlock() {}
//     void unlock_shared() {}
//     bool try_lock() { return true; }
// };
// using Mutex = NullMutex;
// using SharedMutex = NullMutex;
#else
#include <mutex>
#include <shared_mutex>
using Mutex = std::mutex;
using SharedMutex = std::shared_mutex;
#endif

