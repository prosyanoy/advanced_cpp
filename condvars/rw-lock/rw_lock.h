#pragma once
#include <mutex>
#include <condition_variable>

class RWLock {
public:
    template <class Func>
    void Read(Func func) {
        read_.lock();
        ++blocked_readers_;
        if (blocked_readers_ == 1) {
            global_.lock();
        }
        read_.unlock();
        try {
            func();
        } catch (...) {
            EndRead();
            throw;
        }
        EndRead();
    }

    template <class Func>
    void Write(Func func) {
        std::unique_lock<std::mutex> lock(global_);
        cv_.wait(lock, [this] {
            return !blocked_readers_;
        });
        func();
    }

private:
    std::mutex read_;
    std::mutex global_;
    int blocked_readers_ = 0;
    std::condition_variable cv_;

    void EndRead() {
        read_.lock();
        --blocked_readers_;
        if (!blocked_readers_) {
            cv_.notify_one();
            global_.unlock();
        }
        read_.unlock();
    }
};
