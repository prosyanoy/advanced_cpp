#pragma once

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <map>

template <class T>
class TimerQueue {
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::multimap<std::chrono::system_clock::time_point, T> queue_;

public:
    void Add(const T& item, TimePoint at) {
        std::unique_lock<std::mutex> lock(mtx_);
        bool notify = queue_.empty() || at < queue_.begin()->first;
        queue_.emplace(at, item);
        if (notify) {
            cv_.notify_one();
        }
    }

    T Pop() {
        std::unique_lock<std::mutex> lock(mtx_);
        while (true) {
            if (queue_.empty()) {
                cv_.wait(lock);
            } else {
                auto it = queue_.begin();
                auto now = std::chrono::system_clock::now();
                if (it->first <= now) {
                    T value = it->second;
                    queue_.erase(it);
                    return value;
                } else {
                    cv_.wait_until(lock, it->first);
                }
            }
        }
    }
};
