#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>
#include <thread>
#include <future>

class ThreadPool {
private:
  std::vector<std::thread> threads;
  std::queue<std::function<void()>> tasks;
  std::mutex mutex;
  std::condition_variable cv;
  bool stop = false;

public:

  explicit ThreadPool(size_t n) {
    for (size_t i = 0; i < n; i++) {
      threads.emplace_back([this] {
        while(true) {
          std::unique_lock lock(mutex);
          cv.wait(lock, [this] { return stop || !tasks.empty(); });
          if(stop && tasks.empty()) return;
          auto task = std::move(tasks.front());
          tasks.pop();
          lock.unlock();
          task();
        }
      });
    }
  }

  template<class F>
  std::future<void> submit(F&& f) {
    // wrap the forward ref in a packaged_task, so it knows to copy or move, whichever makes more sense
    auto task = std::make_shared<std::packaged_task<void()>>(std::forward<F>(f));
    std::future<void> fut = task->get_future();

    std::unique_lock lock(mutex);
    tasks.push([task]() { (*task)(); });
    lock.unlock();

    cv.notify_one();
    return fut;
  }

  ~ThreadPool() {
    std::unique_lock lock(mutex);
    stop = true;
    lock.unlock();

    cv.notify_all();
    for(auto &t : threads) {
      t.join();
    }
  }

};
