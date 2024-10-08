#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename X>
class WeakPtr {
    X* ptr_;
    ControlBlockBase* cntrl_;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() noexcept : ptr_(nullptr), cntrl_(nullptr) {
    }

    WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
        if (other.cntrl_) {
            other.cntrl_->AddWeak();
        }
    }

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    WeakPtr(const WeakPtr<Y>& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
        if (other.cntrl_) {
            other.cntrl_->AddWeak();
        }
    }

    WeakPtr(WeakPtr&& other) noexcept : ptr_(other.ptr_), cntrl_(other.cntrl_) {
        other.ptr_ = nullptr;
        other.cntrl_ = nullptr;
    }

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    WeakPtr(WeakPtr<Y>&& other) noexcept : ptr_(other.ptr_), cntrl_(other.cntrl_) {
        other.ptr_ = nullptr;
        other.cntrl_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    WeakPtr(const SharedPtr<Y>& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
        if (other.cntrl_) {
            other.cntrl_->AddWeak();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        WeakPtr(other).Swap(*this);
        return *this;
    }

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    WeakPtr& operator=(const WeakPtr<Y>& other) {
        WeakPtr(other).Swap(*this);
        return *this;
    }

    WeakPtr& operator=(const SharedPtr<X>& other) {
        WeakPtr(other).Swap(*this);
        return *this;
    }

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    WeakPtr& operator=(const SharedPtr<Y>& other) {
        WeakPtr(other).Swap(*this);
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        WeakPtr(std::move(other)).Swap(*this);
        return *this;
    }

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    WeakPtr& operator=(WeakPtr<Y>&& other) {
        WeakPtr(std::move(other)).Swap(*this);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        ReleaseWeakCount();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        WeakPtr().Swap(*this);
    }
    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(cntrl_, other.cntrl_);
    }

    void ReleaseWeakCount() {
        if (cntrl_ && cntrl_->RemoveWeak() == 0) {
            if (cntrl_->strong_count == 0) {
                delete cntrl_;
            } else {
                cntrl_ = nullptr;
                // ptr_ = nullptr?
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (cntrl_) {
            return cntrl_->strong_count;
        } else {
            return 0;
        }
    }
    bool Expired() const {
        return UseCount() == 0;
    }
    SharedPtr<X> Lock() const {
        return Expired() ? SharedPtr<X>() : SharedPtr<X>(*this);
    }

    template <typename T>
    friend class SharedPtr;
};
