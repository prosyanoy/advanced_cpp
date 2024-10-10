#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <memory>

// https://en.cppreference.com/w/cpp/memory/shared_ptr

template <typename X>
class SharedPtr {
public:
    X* ptr_;
    ControlBlockBase* cntrl_;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() noexcept : ptr_(nullptr), cntrl_(nullptr) {
    }

    SharedPtr(std::nullptr_t) noexcept : ptr_(nullptr), cntrl_(nullptr) {
    }

    explicit SharedPtr(X* ptr) : ptr_(ptr), cntrl_(new ControlBlockWithPointer<X>(ptr)) {
    }

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    explicit SharedPtr(Y* ptr) : ptr_(ptr), cntrl_(new ControlBlockWithPointer<X>(ptr)) {
    }

    SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
        if (other.cntrl_) {
            other.cntrl_->AddStrong();
        }
    }

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    SharedPtr(const SharedPtr<Y>& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
        if (other.cntrl_) {
            other.cntrl_->AddStrong();
        }
    }

    SharedPtr(SharedPtr&& other) noexcept : ptr_(other.ptr_), cntrl_(other.cntrl_) {
        other.ptr_ = nullptr;
        other.cntrl_ = nullptr;
    }

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    SharedPtr(SharedPtr<Y>&& other) noexcept : ptr_(other.ptr_), cntrl_(other.cntrl_) {
        other.ptr_ = nullptr;
        other.cntrl_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, X* ptr) noexcept : ptr_(ptr), cntrl_(other.cntrl_) {
        if (cntrl_) {
            cntrl_->AddStrong();
        }
    }

    SharedPtr(const SharedPtr& other, X* ptr) noexcept : ptr_(ptr), cntrl_(other.cntrl_) {
        if (cntrl_) {
            cntrl_->AddStrong();
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr

    explicit SharedPtr(const WeakPtr<X>& other) : ptr_(other.ptr_) {
        if (other.cntrl_->IsDestructed()) {
            throw BadWeakPtr();
        }
        cntrl_ = other.cntrl_;
        if (cntrl_) {
            cntrl_->AddStrong();
        }
    }

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    explicit SharedPtr(const WeakPtr<Y>& other) : ptr_(other.ptr_) {
        if (other.cntrl_->IsDestructed()) {
            throw BadWeakPtr();
        }
        cntrl_ = other.cntrl_;
        if (cntrl_) {
            cntrl_->AddStrong();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) noexcept {
        SharedPtr(other).Swap(*this);
        return *this;
    }

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    SharedPtr& operator=(const SharedPtr& other) noexcept {
        SharedPtr(other).Swap(*this);
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        SharedPtr(std::move(other)).Swap(*this);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        ReleaseStrongCount();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        SharedPtr().Swap(*this);
    }
    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    void Reset(Y* ptr) {
        SharedPtr(ptr).Swap(*this);
    }

    void Swap(SharedPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(cntrl_, other.cntrl_);
    }

    void ReleaseStrongCount() {
        if (cntrl_ && cntrl_->RemoveStrong() == 0) {
            if (cntrl_->weak_count == 0) {
                cntrl_->DeleteSelf();
            } else {
                cntrl_->Destruct();
            }
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    X* Get() const {
        return ptr_;
    }
    X& operator*() const {
        return *ptr_;
    }
    X* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (cntrl_) {
            return cntrl_->strong_count;
        } else {
            return 0;
        }
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }

    template <typename T, typename... Args>
    friend SharedPtr<T> MakeShared(Args&&... args);
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.ptr_ == reinterpret_cast<U*>(right.ptr_);
    // cntrl?
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto a = SharedPtr<T>();

    Combined<T>* combined = new Combined<T>(std::forward<Args>(args)...);

    a.ptr_ = &combined->obj;
    a.cntrl_ = &combined->cntrl_;
    return a;
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
