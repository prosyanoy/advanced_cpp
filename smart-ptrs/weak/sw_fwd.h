#pragma once

#include <exception>

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
    // ControlBlockWithPointer() : ControlBlockBase(1, 0), ptr() {
    // }

    ControlBlockWithPointer(Derived* p) : ControlBlockBase(1, 0), ptr(p) {
    }

    bool IsDestructed() override {
        return ptr.IsDestructed();
    }

    Ptr<Derived> ptr;

    void Destruct() override {
        if (!IsDestructed()) {
            ptr.Destruct();
        }
    }

    ~ControlBlockWithPointer() {
        Destruct();
    }
};

class BadWeakPtr : public std::exception {};

template <typename X>
class SharedPtr;

template <typename X>
class WeakPtr;

/*template <typename X>
class SharedPtr {
    SharedPtr();

    SharedPtr(std::nullptr_t);

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    explicit SharedPtr(Y* ptr);

    SharedPtr(const SharedPtr& other);

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    SharedPtr(const SharedPtr<Y>& other);

    SharedPtr(SharedPtr&& other);

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    SharedPtr(SharedPtr<Y>&& other);

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, X* ptr);

    SharedPtr(const SharedPtr& other, X* ptr);

    explicit SharedPtr(const WeakPtr<X>& other);

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    explicit SharedPtr(const WeakPtr<Y>& other);

    SharedPtr& operator=(const SharedPtr& other);

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    SharedPtr& operator=(const SharedPtr& other);

    SharedPtr& operator=(SharedPtr&& other);

    ~SharedPtr();

    void Reset();

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    void Reset(Y* ptr);

    void Swap(SharedPtr& other);

    void ReleaseStrongCount();

    X* Get() const;

    X& operator*() const;

    X* operator->() const;

    size_t UseCount() const;

    explicit operator bool() const;
};

template <typename X>
class WeakPtr {
    WeakPtr();

    WeakPtr(const WeakPtr& other);
    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    WeakPtr(const WeakPtr<Y>& other);

    WeakPtr(WeakPtr&& other);
    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    WeakPtr(WeakPtr<Y>&& other);

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    WeakPtr(const SharedPtr<Y>& other);

    WeakPtr& operator=(const WeakPtr& other);

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    WeakPtr& operator=(const WeakPtr<Y>& other);

    WeakPtr& operator=(const SharedPtr<X>& other);

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    WeakPtr& operator=(const SharedPtr<Y>& other);

    WeakPtr& operator=(WeakPtr&& other);

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    WeakPtr& operator=(WeakPtr<Y>&& other);

    ~WeakPtr();

    void Reset();

    void Swap(WeakPtr& other);

    void ReleaseWeakCount();

    size_t UseCount() const;

    bool Expired() const;

    SharedPtr<X> Lock() const;
};
*/