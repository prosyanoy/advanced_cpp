#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <memory>

// https://en.cppreference.com/w/cpp/memory/shared_ptr


template <typename T>
class PtrInternalInterface {
public:
    virtual ~PtrInternalInterface() = 0;

    virtual T* Get() = 0;
    virtual void Destruct() = 0;
}; // class PtrInternalInterface

template <typename T>
PtrInternalInterface<T>::~PtrInternalInterface() {}


template <typename T, typename D>
class PtrInternal: public PtrInternalInterface<T> {
private:
    T* ptr_;
    D deleter_;

public:
    PtrInternal(T* p, D d): ptr_(p), deleter_(std::move(d)) {}
    ~PtrInternal() override {
        Destruct();
    }

    T* Get() override {
        return ptr_;
    }

    void Destruct() override {
        deleter_(ptr_);
    }
}; // class PtrInternal

template <typename T>
class Ptr {
    template <typename U>
    struct DefaultDeleter {
        void operator()(T* t) {
            if (t) {
                delete static_cast<U*>(t);
            }
        }
    };

    template <typename Derived>
    using DefaultInternal = PtrInternal<T, DefaultDeleter<Derived>>;

    PtrInternalInterface<T>* internal_;

public:
    template <typename Derived>
    Ptr(Derived* d) : internal_(new DefaultInternal<Derived>{d, DefaultDeleter<Derived>()}) {}

    T* Get() {
        return internal_->Get();
    }
    void Destruct() {
        internal_->Destruct();
    }
    ~Ptr() {
        delete internal_;
    }
}; // class Ptr

struct ControlBlockBase {
    int counter;
    ControlBlockBase(int c) : counter(c) {
    }
    virtual ~ControlBlockBase() = default;
};

template <typename T>
struct ControlBlockWithObject : ControlBlockBase {
    T obj;

    template <typename... Args>
    ControlBlockWithObject(Args&&... args) : ControlBlockBase(1), obj(std::forward<Args>(args)...) {
    }

    ~ControlBlockWithObject() = default;
};

template <class Derived>
struct ControlBlockWithPointer : ControlBlockBase {
    ControlBlockWithPointer(Derived* p) : ControlBlockBase(1), ptr(p) {
    }

    Ptr<Derived> ptr;

    ~ControlBlockWithPointer() = default;
};

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

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    explicit SharedPtr(Y* ptr) : ptr_(ptr), cntrl_(new ControlBlockWithPointer<Y>(ptr)) {
    }

    SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
        if (other.cntrl_) {
            ++other.cntrl_->counter;
        }
    }

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    SharedPtr(const SharedPtr<Y>& other) : ptr_(other.ptr_), cntrl_(other.cntrl_) {
        if (other.cntrl_) {
            ++other.cntrl_->counter;
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
            ++cntrl_->counter;
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    /*
    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    explicit SharedPtr(const WeakPtr<Y>& other) : ptr_(other.ptr_) {
        if (!other.cntrl_) {
            throw std::exception(); // bad_weak_ptr()
        }
        cntrl_ = other.cntrl_;
    }
     */

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
        ReleaseCount();
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

    void ReleaseCount() {
        if (cntrl_ && --cntrl_->counter == 0) {
            delete cntrl_;
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
            return cntrl_->counter;
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
    auto cntrl = new ControlBlockWithObject<T>(std::forward<Args>(args)...);
    a.ptr_ = &cntrl->obj;
    a.cntrl_ = cntrl;
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
