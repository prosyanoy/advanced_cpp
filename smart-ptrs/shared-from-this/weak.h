#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr

template <typename X>
inline WeakPtr<X>::WeakPtr() noexcept : ptr_(nullptr), cntrl_(nullptr) {
}

template <typename X>
inline WeakPtr<X>::WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    if (other.cntrl_) {
        other.cntrl_->AddWeak();
    }
}

template <typename X>
template <class Y, class>
inline WeakPtr<X>::WeakPtr(const WeakPtr<Y>& other) : cntrl_(other.cntrl_) {
    ptr_ = static_cast<X*>(other.ptr_);
    if (other.cntrl_) {
        other.cntrl_->AddWeak();
    }
}

template <typename X>
inline WeakPtr<X>::WeakPtr(WeakPtr&& other) noexcept : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    other.ptr_ = nullptr;
    other.cntrl_ = nullptr;
}

template <typename X>
template <class Y, class>
inline WeakPtr<X>::WeakPtr(WeakPtr<Y>&& other) noexcept : cntrl_(other.cntrl_) {
    ptr_ = static_cast<X*>(other.ptr_);
    other.ptr_ = nullptr;
    other.cntrl_ = nullptr;
}

// Demote `SharedPtr`
// #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
template <typename X>
template <class Y, class>
inline WeakPtr<X>::WeakPtr(const SharedPtr<Y>& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    if (other.cntrl_) {
        other.cntrl_->AddWeak();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
// `operator=`-s

template <typename X>
inline WeakPtr<X>& WeakPtr<X>::operator=(const WeakPtr& other) {
    WeakPtr(other).Swap(*this);
    return *this;
}

template <typename X>
template <class Y, class>
inline WeakPtr<X>& WeakPtr<X>::operator=(const WeakPtr<Y>& other) {
    WeakPtr(other).Swap(*this);
    return *this;
}

template <typename X>
inline WeakPtr<X>& WeakPtr<X>::operator=(const SharedPtr<X>& other) {
    WeakPtr(other).Swap(*this);
    return *this;
}

template <typename X>
template <class Y, class>
inline WeakPtr<X>& WeakPtr<X>::operator=(const SharedPtr<Y>& other) {
    WeakPtr(other).Swap(*this);
    return *this;
}

template <typename X>
inline WeakPtr<X>& WeakPtr<X>::operator=(WeakPtr&& other) {
    WeakPtr(std::move(other)).Swap(*this);
    return *this;
}

template <typename X>
template <class Y, class>
inline WeakPtr<X>& WeakPtr<X>::operator=(WeakPtr<Y>&& other) {
    WeakPtr(std::move(other)).Swap(*this);
    return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor

template <typename X>
inline WeakPtr<X>::~WeakPtr() {
    ReleaseWeakCount();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Modifiers

template <typename X>
inline void WeakPtr<X>::Reset() {
    WeakPtr().Swap(*this);
}

template <typename X>
inline void WeakPtr<X>::Swap(WeakPtr& other) {
    std::swap(ptr_, other.ptr_);
    std::swap(cntrl_, other.cntrl_);
}

template <typename X>
inline void WeakPtr<X>::ReleaseWeakCount() {
    if (cntrl_) {
        cntrl_->RemoveWeak();
        if (cntrl_->weak_count == 0) {
            Release();
        }
    }
}

template <typename X>
inline void WeakPtr<X>::Release() {
    if (cntrl_->strong_count == 0) {
        delete cntrl_;
    } else {
        cntrl_ = nullptr;
        // ptr_ = nullptr?
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Observers

template <typename X>
inline size_t WeakPtr<X>::UseCount() const {
    if (cntrl_) {
        return cntrl_->strong_count;
    } else {
        return 0;
    }
}

template <typename X>
inline bool WeakPtr<X>::Expired() const {
    return UseCount() == 0;
}

template <typename X>
inline SharedPtr<X> WeakPtr<X>::Lock() const {
    return Expired() ? SharedPtr<X>() : SharedPtr<X>(*this);
}

/////////////////////////////////////////////////////////////////////////////
// VOID

inline WeakPtr<void>::WeakPtr() noexcept : ptr_(nullptr), cntrl_(nullptr) {
}

inline WeakPtr<void>::WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    if (other.cntrl_) {
        other.cntrl_->AddWeak();
    }
}

template <class Y>
inline WeakPtr<void>::WeakPtr(const WeakPtr<Y>& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    if (other.cntrl_) {
        other.cntrl_->AddWeak();
    }
}

inline WeakPtr<void>::WeakPtr(WeakPtr&& other) noexcept : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    other.ptr_ = nullptr;
    other.cntrl_ = nullptr;
}

template <class Y>
inline WeakPtr<void>::WeakPtr(WeakPtr<Y>&& other) noexcept
    : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    other.ptr_ = nullptr;
    other.cntrl_ = nullptr;
}

// Demote `SharedPtr`
// #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
template <class Y>
inline WeakPtr<void>::WeakPtr(const SharedPtr<Y>& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    if (other.cntrl_) {
        other.cntrl_->AddWeak();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
// `operator=`-s

inline WeakPtr<void>& WeakPtr<void>::operator=(const WeakPtr& other) {
    WeakPtr(other).Swap(*this);
    return *this;
}

template <class Y>
inline WeakPtr<void>& WeakPtr<void>::operator=(const WeakPtr<Y>& other) {
    WeakPtr(other).Swap(*this);
    return *this;
}

inline WeakPtr<void>& WeakPtr<void>::operator=(const SharedPtr<void>& other) {
    WeakPtr(other).Swap(*this);
    return *this;
}

template <class Y>
inline WeakPtr<void>& WeakPtr<void>::operator=(const SharedPtr<Y>& other) {
    WeakPtr(other).Swap(*this);
    return *this;
}

inline WeakPtr<void>& WeakPtr<void>::operator=(WeakPtr&& other) {
    WeakPtr(std::move(other)).Swap(*this);
    return *this;
}

template <class Y>
inline WeakPtr<void>& WeakPtr<void>::operator=(WeakPtr<Y>&& other) {
    WeakPtr(std::move(other)).Swap(*this);
    return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor

inline WeakPtr<void>::~WeakPtr() {
    ReleaseWeakCount();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Modifiers

inline void WeakPtr<void>::Reset() {
    WeakPtr().Swap(*this);
}

inline void WeakPtr<void>::Swap(WeakPtr& other) {
    std::swap(ptr_, other.ptr_);
    std::swap(cntrl_, other.cntrl_);
}

inline void WeakPtr<void>::ReleaseWeakCount() {
    if (cntrl_) {
        cntrl_->RemoveWeak();
        if (cntrl_->weak_count == 0) {
            Release();
        }
    }
}

inline void WeakPtr<void>::Release() {
    if (cntrl_->strong_count == 0) {
        delete cntrl_;
    } else {
        cntrl_ = nullptr;
        // ptr_ = nullptr?
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Observers

inline size_t WeakPtr<void>::UseCount() const {
    if (cntrl_) {
        return cntrl_->strong_count;
    } else {
        return 0;
    }
}

inline bool WeakPtr<void>::Expired() const {
    return UseCount() == 0;
}

inline SharedPtr<void> WeakPtr<void>::Lock() const {
    return Expired() ? SharedPtr<void>() : SharedPtr<void>(*this);
}
