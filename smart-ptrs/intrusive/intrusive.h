#pragma once

#include <cstddef> // for std::nullptr_t
#include <utility> // for std::exchange / std::swap
#include <exception>


class SimpleCounter {
    size_t count_ = 0;

public:
    size_t IncRef() {
        return ++count_;
    }
    size_t DecRef() {
        if (count_ == 0 || count_ == 1) {
            return 0;
        } else {
            return --count_;
        }
    }
    size_t RefCount() const {
        return count_;
    }
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
    Counter counter_ = Counter();

public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    }

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        if (!counter_.DecRef()) {
            Deleter().Destroy(static_cast<Derived*>(this));
        }
    }

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    }
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;
    T* ptr_;
public:
    // Constructors
    IntrusivePtr() : ptr_(nullptr) {
    }
    IntrusivePtr(std::nullptr_t) : ptr_(nullptr) {
    }
    IntrusivePtr(T* ptr) : ptr_(ptr) {
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    template <typename Y, class = std::enable_if_t<std::is_convertible<Y*, T*>::value>>
    IntrusivePtr(const IntrusivePtr<Y>& other) : ptr_(other.ptr_) {
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    template <typename Y, class = std::enable_if_t<std::is_convertible<Y*, T*>::value>>
    IntrusivePtr(IntrusivePtr<Y>&& other) : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    IntrusivePtr(const IntrusivePtr& other) : ptr_(other.ptr_) {
        if (ptr_) {
            ptr_->IncRef();
        }
    }
    IntrusivePtr(IntrusivePtr&& other) : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    // `operator=`-s
    template <typename Y, class = std::enable_if_t<std::is_convertible<Y*, T*>::value>>
    IntrusivePtr& operator=(const IntrusivePtr<Y>& other) {
        IntrusivePtr(other).Swap(*this);
        return *this;
    }
    template <typename Y, class = std::enable_if_t<std::is_convertible<Y*, T*>::value>>
    IntrusivePtr& operator=(IntrusivePtr<Y>&& other) {
        IntrusivePtr(std::move(other)).Swap(*this);
        return *this;
    }

    IntrusivePtr& operator=(const IntrusivePtr& other) {
        IntrusivePtr(other).Swap(*this);
        return *this;
    }
    IntrusivePtr& operator=(IntrusivePtr&& other) {
        IntrusivePtr(std::move(other)).Swap(*this);
        return *this;
    }

    // Destructor
    ~IntrusivePtr() {
        if (ptr_) {
            ptr_->DecRef();
        }
    }

    // Modifiers
    void Reset() {
        IntrusivePtr().Swap(*this);
    }
    void Reset(T* ptr) {
        IntrusivePtr(ptr).Swap(*this);
    }
    void Swap(IntrusivePtr& other) {
        std::swap(ptr_, other.ptr_);
    }

    // Observers
    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (ptr_) {
            return ptr_->RefCount();
        } else {
            return 0;
        }
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }
};

template <typename T, typename ...Args>
IntrusivePtr<T> MakeIntrusive(Args&& ...args) {
    return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
}
