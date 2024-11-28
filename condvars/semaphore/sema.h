#pragma once

#include <mutex>
#include <condition_variable>

class DefaultCallback {
public:
    void operator()(int& value) {
        --value;
    }
};

class Semaphore {
public:
    Semaphore(int count) : count_(count) {
    }

    void Leave() {
        bool was_empty = false;
        {
            std::lock_guard guard{mutex_};
            ++count_;
            was_empty = (count_ == 1);
        }
        if (was_empty) {
            cv_.notify_one();
        }
    }

    template <class Func>
    void Enter(Func callback) {
        int order = counter1_++;
        std::unique_lock<std::mutex> guard(mutex_);
        cv_.wait(guard, [this, order] { return count_ > 0 && counter2_ == order; });
        callback(count_);
        ++counter2_;
    }

    void Enter() {
        DefaultCallback callback;
        Enter(callback);
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    int count_ = 0;
    std::atomic<int> counter1_ = 0;
    std::atomic<int> counter2_ = 0;
};
