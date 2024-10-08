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
};  // class PtrInternalInterface

template <typename T>
PtrInternalInterface<T>::~PtrInternalInterface() {
}

template <typename T, typename D>
class PtrInternal : public PtrInternalInterface<T> {
private:
    T* ptr_;
    D deleter_;

public:
    PtrInternal(T* p, D d) : ptr_(p), deleter_(std::move(d)) {
    }

    ~PtrInternal() override {
        Destruct();
    }

    T* Get() override {
        return ptr_;
    }

    void Destruct() override {
        if (ptr_) {
            deleter_(ptr_);
            ptr_ = nullptr;
        }
    }
};  // class PtrInternal

template <typename T>
class Ptr {
    template <typename U>
    struct DefaultDeleter {
        void operator()(T* t) {
            delete static_cast<U*>(t);
        }
    };

    template <typename Derived>
    using DefaultInternal = PtrInternal<T, DefaultDeleter<Derived>>;

    PtrInternalInterface<T>* internal_;

public:
    template <typename Derived>
    Ptr(Derived* d) : internal_(new DefaultInternal<Derived>{d, DefaultDeleter<Derived>()}) {
    }

    bool IsDestructed() {
        return internal_->Get() == nullptr;
    }
    T* Get() {
        return internal_->Get();
    }
    void Destruct() {
        internal_->Destruct();
    }
    ~Ptr() {
        delete internal_;
    }
};  // class Ptr

struct ControlBlockBase {
    int strong_count;
    int weak_count;

    ControlBlockBase(int strong, int weak) : strong_count(strong), weak_count(weak) {
    }
    int AddStrong() {
        return ++strong_count;
    }
    int AddWeak() {
        return ++weak_count;
    }
    int RemoveStrong() {
        return --strong_count;
    }
    int RemoveWeak() {
        return --weak_count;
    }

    virtual bool IsDestructed() = 0;
    virtual void Destruct() = 0;
    virtual ~ControlBlockBase() = default;
};

template <typename Derived>
struct ControlBlockWithObject : ControlBlockBase {
    Derived* ptr;

    ControlBlockWithObject(Derived* p) : ControlBlockBase(1, 0), ptr(p) {
    }

    bool hasBeenDestructed;

    bool IsDestructed() override {
        return hasBeenDestructed;
    }

    ~ControlBlockWithObject() {
        if (!IsDestructed()) {
            Destruct();
        }
    }

    void Destruct() override {
        ptr->~Derived();
        hasBeenDestructed = true;
    }
};

template <class Derived>
struct ControlBlockWithPointer : ControlBlockBase {
    //ControlBlockWithPointer() : ControlBlockBase(1, 0), ptr() {
    //}

    ControlBlockWithPointer(Derived* p) : ControlBlockBase(1, 0), ptr(p) {
    }

    bool IsDestructed() override {
        return ptr.IsDestructed();
    }

    Ptr<Derived> ptr;

    void Destruct() {
        if (!IsDestructed()) {
            ptr.Destruct();
        }
    }

    ~ControlBlockWithPointer() {
        Destruct();
    }
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
                delete cntrl_;
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

template <typename T>
struct Combined {
    ControlBlockWithObject<T> cntrl_;
    T obj;
    template <typename... Args>
    Combined(Args&&... args) : obj(std::forward<Args>(args)...), cntrl_(&obj) {
    }
    ~Combined() = delete;
};

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
