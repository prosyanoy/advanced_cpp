#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <memory>

// https://en.cppreference.com/w/cpp/memory/shared_ptr

class ESFTBase {
    mutable WeakPtr<void> weak_this_;

public:
    virtual void SetWeakThis(void* ptr, ControlBlockBase* cntrl) const {
        weak_this_.ptr_ = ptr;
        weak_this_.cntrl_ = cntrl;
    }

    template <typename T>
    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(weak_this_);
    }

    template <typename T>
    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<T>(weak_this_);
    }

    template <typename T>
    WeakPtr<T> WeakFromThis() noexcept {
        return weak_this_;
    }

    template <typename T>
    WeakPtr<const T> WeakFromThis() const noexcept {
        return weak_this_;
    }

    virtual ~ESFTBase() {
        SetWeakThis(nullptr, nullptr);
    }
};

template <typename X>
class EnableSharedFromThis : public ESFTBase {
public:
    SharedPtr<X> SharedFromThis() {
        return ESFTBase::SharedFromThis<X>();
    }
    SharedPtr<const X> SharedFromThis() const {
        return ESFTBase::SharedFromThis<X>();
    }

    WeakPtr<X> WeakFromThis() noexcept {
        return ESFTBase::WeakFromThis<X>();
    }
    WeakPtr<const X> WeakFromThis() const noexcept {
        return ESFTBase::WeakFromThis<X>();
    }
};



////////////////////////////////////////////////////////////////////////////////////////////////
// Constructors

template <typename X>
inline SharedPtr<X>::SharedPtr() noexcept : ptr_(nullptr), cntrl_(nullptr) {
}

template <typename X>
inline SharedPtr<X>::SharedPtr(std::nullptr_t) noexcept : ptr_(nullptr), cntrl_(nullptr) {
}

template <typename X>
inline constexpr SharedPtr<X>::SharedPtr(X* ptr) {
    ptr_ = ptr;
    cntrl_ = new ControlBlockWithPointer<X>(ptr);
    if constexpr (std::is_convertible_v<X*, ESFTBase*>) {
        if (auto p = dynamic_cast<ESFTBase*>(ptr)) {
            p->SetWeakThis(ptr, cntrl_);
        }
    }
}

template <typename X>
template <class Y, class>
inline constexpr SharedPtr<X>::SharedPtr(Y* ptr) {
    ptr_ = ptr;
    cntrl_ = new ControlBlockWithPointer<X>(ptr);
    if constexpr (std::is_convertible_v<Y*, ESFTBase*>) {
        if (auto p = dynamic_cast<ESFTBase*>(ptr)) {
            p->SetWeakThis(ptr, cntrl_);
        }
    }
}

template <typename X>
inline SharedPtr<X>::SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    if (other.cntrl_) {
        other.cntrl_->AddStrong();
    }
}

template <typename X>
template <class Y, class>
inline SharedPtr<X>::SharedPtr(const SharedPtr<Y>& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    if (other.cntrl_) {
        other.cntrl_->AddStrong();
    }
}

template <typename X>
inline SharedPtr<X>::SharedPtr(SharedPtr&& other) noexcept : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    other.ptr_ = nullptr;
    other.cntrl_ = nullptr;
}

template <typename X>
template <class Y, class>
inline SharedPtr<X>::SharedPtr(SharedPtr<Y>&& other) noexcept : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    other.ptr_ = nullptr;
    other.cntrl_ = nullptr;
}

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
template <typename X>
template <class Y>
inline SharedPtr<X>::SharedPtr(const SharedPtr<Y>& other, X* ptr) noexcept : ptr_(ptr), cntrl_(other.cntrl_) {
    if (cntrl_) {
        cntrl_->AddStrong();
    }
}

template <typename X>
inline SharedPtr<X>::SharedPtr(const SharedPtr& other, X* ptr) noexcept : ptr_(ptr), cntrl_(other.cntrl_) {
    if (cntrl_) {
        cntrl_->AddStrong();
    }
}

// Promote `WeakPtr`
// #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr

template <typename X>
inline SharedPtr<X>::SharedPtr(const WeakPtr<X>& other) {
    if (other.cntrl_->IsDestructed()) {
        throw BadWeakPtr();
    }
    ptr_ = static_cast<X*>(other.ptr_);
    cntrl_ = other.cntrl_;
    cntrl_->AddStrong();
}

template <typename X>
template <class Y, class>
inline SharedPtr<X>::SharedPtr(const WeakPtr<Y>& other) {
    if (!other.cntrl_ || other.cntrl_->IsDestructed()) {
        throw BadWeakPtr();
    }
    ptr_ = static_cast<X*>(other.ptr_);
    cntrl_ = other.cntrl_;
    cntrl_->AddStrong();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// `operator=`-s

template <typename X>
inline SharedPtr<X>& SharedPtr<X>::operator=(const SharedPtr<X>& other) noexcept {
    SharedPtr(other).Swap(*this);
    return *this;
}

template <typename X>
template <class Y, class>
inline SharedPtr<X>& SharedPtr<X>::operator=(const SharedPtr<Y>& other) noexcept {
    SharedPtr(other).Swap(*this);
    return *this;
}


template <typename X>
inline SharedPtr<X>& SharedPtr<X>::operator=(SharedPtr&& other) noexcept {
    SharedPtr(std::move(other)).Swap(*this);
    return *this;
}

template <typename X>
template <class Y, class>
inline SharedPtr<X>& SharedPtr<X>::operator=(SharedPtr<Y>&& other) noexcept {
    SharedPtr(std::move(other)).Swap(*this);
    return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor

template <typename X>
inline SharedPtr<X>::~SharedPtr() {
    ReleaseStrongCount();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Modifiers

template <typename X>
inline void SharedPtr<X>::Reset() {
    SharedPtr().Swap(*this);
}

template <typename X>
template <class Y, class>
inline void SharedPtr<X>::Reset(Y* ptr) {
    SharedPtr(ptr).Swap(*this);
}

template <typename X>
inline void SharedPtr<X>::Swap(SharedPtr& other) {
    std::swap(ptr_, other.ptr_);
    std::swap(cntrl_, other.cntrl_);
}

template <typename X>
inline void SharedPtr<X>::ReleaseStrongCount() {
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

template <typename X>
inline X* SharedPtr<X>::Get() const {
    return ptr_;
}

template <typename X>
inline X& SharedPtr<X>::operator*() const {
    return *ptr_;
}

template <typename X>
inline X* SharedPtr<X>::operator->() const {
    return ptr_;
}

template <typename X>
inline size_t SharedPtr<X>::UseCount() const {
    if (cntrl_) {
        return cntrl_->strong_count;
    } else {
        return 0;
    }
}

template <typename X>
inline SharedPtr<X>::operator bool() const {
    return ptr_ != nullptr;
}

/////////////////////////////////////////////////////////////////////////////
// VOID

inline SharedPtr<void>::SharedPtr() noexcept : ptr_(nullptr), cntrl_(nullptr) {
}

inline SharedPtr<void>::SharedPtr(std::nullptr_t) noexcept : ptr_(nullptr), cntrl_(nullptr) {
}

inline SharedPtr<void>::SharedPtr(void* ptr) {
    ptr_ = ptr;
    cntrl_ = new ControlBlockWithPointer<void>(ptr);
}

template <class Y>
inline SharedPtr<void>::SharedPtr(Y* ptr) {
    ptr_ = ptr;
    cntrl_ = new ControlBlockWithPointer<void>(ptr);
}

inline SharedPtr<void>::SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    if (other.cntrl_) {
        other.cntrl_->AddStrong();
    }
}

template <class Y>
inline SharedPtr<void>::SharedPtr(const SharedPtr<Y>& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    if (other.cntrl_) {
        other.cntrl_->AddStrong();
    }
}

inline SharedPtr<void>::SharedPtr(SharedPtr&& other) noexcept : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    other.ptr_ = nullptr;
    other.cntrl_ = nullptr;
}

template <class Y>
inline SharedPtr<void>::SharedPtr(SharedPtr<Y>&& other) noexcept : ptr_(other.ptr_), cntrl_(other.cntrl_) {
    other.ptr_ = nullptr;
    other.cntrl_ = nullptr;
}

// Aliasing constructor
// #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
template <class Y>
inline SharedPtr<void>::SharedPtr(const SharedPtr<Y>& other, void* ptr) noexcept : ptr_(ptr), cntrl_(other.cntrl_) {
    if (cntrl_) {
        cntrl_->AddStrong();
    }
}

// Promote `WeakPtr`
// #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr

inline SharedPtr<void>::SharedPtr(const WeakPtr<void>& other) : ptr_(other.ptr_) {
    if (other.cntrl_->IsDestructed()) {
        throw BadWeakPtr();
    }
    cntrl_ = other.cntrl_;
    if (cntrl_) {
        cntrl_->AddStrong();
    }
}

template <class Y>
inline SharedPtr<void>::SharedPtr(const WeakPtr<Y>& other) {
    if (!other.cntrl_ || other.cntrl_->IsDestructed()) {
        throw BadWeakPtr();
    }
    ptr_ = static_cast<void*>(other.ptr_);
    cntrl_ = other.cntrl_;
    cntrl_->AddStrong();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// `operator=`-s

template <class Y>
inline SharedPtr<void>& SharedPtr<void>::operator=(const SharedPtr<Y>& other) noexcept {
    SharedPtr(other).Swap(*this);
    return *this;
}


inline SharedPtr<void>& SharedPtr<void>::operator=(SharedPtr&& other) noexcept {
    SharedPtr(std::move(other)).Swap(*this);
    return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor

inline SharedPtr<void>::~SharedPtr() {
    ReleaseStrongCount();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Modifiers

inline void SharedPtr<void>::Reset() {
    SharedPtr().Swap(*this);
}

template <class Y>
inline void SharedPtr<void>::Reset(Y* ptr) {
    SharedPtr(ptr).Swap(*this);
}

inline void SharedPtr<void>::Swap(SharedPtr& other) {
    std::swap(ptr_, other.ptr_);
    std::swap(cntrl_, other.cntrl_);
}

inline void SharedPtr<void>::ReleaseStrongCount() {
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

inline void* SharedPtr<void>::Get() const {
    return ptr_;
}

inline void* SharedPtr<void>::operator->() const {
    return ptr_;
}

inline size_t SharedPtr<void>::UseCount() const {
    if (cntrl_) {
        return cntrl_->strong_count;
    } else {
        return 0;
    }
}

inline SharedPtr<void>::operator bool() const {
    return ptr_ != nullptr;
}

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
    if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
        if (auto p = dynamic_cast<ESFTBase*>(a.ptr_)) {
            p->SetWeakThis(a.ptr_, a.cntrl_);
        }
    }
    return a;
}