#pragma once
#include <utility>

template <typename Callback>
class CallbackStorage {
    alignas(Callback) char callback_buffer_[sizeof(Callback)];

public:
    bool IsInitialized = false;

    explicit CallbackStorage(Callback callback) {
        ::new (static_cast<void*>(callback_buffer_)) Callback(std::move(callback));
        IsInitialized = true;
    }

    void* GetCallbackBuffer() {
        return static_cast<void*>(callback_buffer_);
    }

    Callback& GetCallback() {
        return *reinterpret_cast<Callback*>(GetCallbackBuffer());
    }

    void Clean() {
        if (IsInitialized) {
            GetCallback().~Callback();
            IsInitialized = false;
        }
    }

    ~CallbackStorage() {
        Clean();
    }
};

template <typename Callback>
class Defer final {
    CallbackStorage<Callback> cs_;

public:
    Defer(Callback&& cb) : cs_(std::move(cb)) {
    }

    void Cancel() {
        cs_.Clean();
    }

    void Invoke() {
        if (cs_.IsInitialized) {
            std::move(cs_.GetCallback())();
            cs_.Clean();
        }
    }

    ~Defer() {
        Invoke();
    }
};
