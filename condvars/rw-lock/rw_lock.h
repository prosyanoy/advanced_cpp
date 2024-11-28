#include <mutex>
#include <condition_variable>

class RWLock {
public:
    template <class Func>
    void Read(Func func) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_readers_.wait(lock, [this]() { return !writer_active_ && writer_waiting_ == 0; });
        ++reader_count_;
        lock.unlock();

        func();

        lock.lock();
        --reader_count_;
        if (reader_count_ == 0 && writer_waiting_ > 0) {
            cv_writers_.notify_one();
        }
        lock.unlock();
    }

    template <class Func>
    void Write(Func func) {
        std::unique_lock<std::mutex> lock(mutex_);
        ++writer_waiting_;
        cv_writers_.wait(lock, [this]() { return !writer_active_ && reader_count_ == 0; });
        --writer_waiting_;
        writer_active_ = true;
        lock.unlock();

        func();

        lock.lock();
        writer_active_ = false;
        if (writer_waiting_ > 0) {
            cv_writers_.notify_one();
        } else {
            cv_readers_.notify_all();
        }
        lock.unlock();
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_readers_;
    std::condition_variable cv_writers_;
    int reader_count_ = 0;
    int writer_waiting_ = 0;
    bool writer_active_ = false;
};
