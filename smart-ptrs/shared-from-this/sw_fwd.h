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

template <class X>
struct ControlBlockWithObject;

template <typename X>
struct Combined {
    X obj;
    ControlBlockWithObject<X> cntrl_;
    template <typename... Args>
    Combined(Args&&... args) : obj(std::forward<Args>(args)...), cntrl_(&obj) {
    }
    ~Combined() = delete;
};  // struct Combined

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
    virtual void DeleteSelf() = 0;
};

template <typename X>
struct ControlBlockWithObject : ControlBlockBase {
    X* ptr;
    bool hasBeenDestructed;

    ControlBlockWithObject(X* p) : ControlBlockBase(1, 0), ptr(p), hasBeenDestructed(false) {
    }

    bool IsDestructed() override {
        return hasBeenDestructed;
    }

    void Destruct() override {
        if (!IsDestructed()) {
            ptr->~X();
            hasBeenDestructed = true;
        }
    }

    void DeleteSelf() override {
        Destruct();
        delete this;
    }

    static void operator delete(void* ptr) {
        ::operator delete(reinterpret_cast<char*>(ptr) - offsetof(Combined<X>, cntrl_));
    }
};

template <class X>
struct ControlBlockWithPointer : ControlBlockBase {
    Ptr<X> ptr;

    ControlBlockWithPointer(X* p) : ControlBlockBase(1, 0), ptr(p) {
    }

    template <class Y>
    ControlBlockWithPointer(Y* p) : ControlBlockBase(1, 0), ptr(p) {
    }

    bool IsDestructed() override {
        return ptr.IsDestructed();
    }

    void Destruct() override {
        if (!IsDestructed()) {
            ptr.Destruct();
        }
    }

    void DeleteSelf() override {
        delete this;
    }

    ~ControlBlockWithPointer() {
        Destruct();
    }
};

class BadWeakPtr : public std::exception {};

template <typename X>
class WeakPtr;

template<>
class WeakPtr<void>;

template <typename X>
class SharedPtr {
public:
    X* ptr_;
    ControlBlockBase* cntrl_;

    SharedPtr() noexcept;
    SharedPtr(std::nullptr_t) noexcept;
    constexpr explicit SharedPtr(X* ptr);

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    constexpr explicit SharedPtr(Y* ptr);

    SharedPtr(const SharedPtr& other);

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    SharedPtr(const SharedPtr<Y>& other);

    SharedPtr(SharedPtr&& other) noexcept;

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    SharedPtr(SharedPtr<Y>&& other) noexcept;

    template <class Y>
    SharedPtr(const SharedPtr<Y>& other, X* ptr) noexcept;

    SharedPtr(const SharedPtr& other, X* ptr) noexcept;

    explicit SharedPtr(const WeakPtr<X>& other);

    template <class Y, class = std::enable_if_t<std::is_convertible_v<Y*, X*> || std::is_same_v<Y, void>>>
    explicit SharedPtr(const WeakPtr<Y>& other);

    SharedPtr<X>& operator=(const SharedPtr<X>& other) noexcept;

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    SharedPtr& operator=(const SharedPtr<Y>& other) noexcept;

    SharedPtr<X>& operator=(SharedPtr&& other) noexcept;

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    SharedPtr<X>& operator=(SharedPtr<Y>&& other) noexcept;

    // Destructor
    ~SharedPtr();

    void Reset();
    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    void Reset(Y* ptr);
    void Swap(SharedPtr& other);

    void ReleaseStrongCount();

    // Observers
    X* Get() const;
    X& operator*() const;
    X* operator->() const;
    size_t UseCount() const;
    explicit operator bool() const;
};

template <>
class SharedPtr<void> {
public:
    void* ptr_;
    ControlBlockBase* cntrl_;

    SharedPtr() noexcept;
    SharedPtr(std::nullptr_t) noexcept;
    explicit SharedPtr(void* ptr);

    template <class Y>
    explicit SharedPtr(Y* ptr);

    SharedPtr(const SharedPtr& other);

    template <class Y>
    SharedPtr(const SharedPtr<Y>& other);

    SharedPtr(SharedPtr&& other) noexcept;

    template <class Y>
    SharedPtr(SharedPtr<Y>&& other) noexcept;

    template <class Y>
    SharedPtr(const SharedPtr<Y>& other, void* ptr) noexcept;

    explicit SharedPtr(const WeakPtr<void>& other);

    template <class Y>
    explicit SharedPtr(const WeakPtr<Y>& other);

    template <class Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) noexcept;

    SharedPtr<void>& operator=(SharedPtr&& other) noexcept;

    // Destructor
    ~SharedPtr();

    void Reset();
    template <class Y>
    void Reset(Y* ptr);
    void Swap(SharedPtr& other);

    void ReleaseStrongCount();

    // Observers
    void* Get() const;
    void* operator->() const;
    size_t UseCount() const;
    explicit operator bool() const;
};

template <typename X>
class WeakPtr {
public:
    X* ptr_;
    ControlBlockBase* cntrl_;

    WeakPtr() noexcept;

    WeakPtr(const WeakPtr& other);

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value || std::is_same_v<Y, void>>>
    WeakPtr(const WeakPtr<Y>& other);

    WeakPtr(WeakPtr&& other) noexcept;

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value || std::is_same_v<Y, void>>>
    WeakPtr(WeakPtr<Y>&& other) noexcept;

    template <class Y, class = std::enable_if_t<std::is_convertible<Y*, X*>::value>>
    WeakPtr(const SharedPtr<Y>& other);

    WeakPtr& operator=(const WeakPtr& other);

    template <class Y,
              class = std::enable_if_t<std::is_convertible_v<Y*, X*> &&
                                       !std::is_same_v<Y, void>>>
    WeakPtr& operator=(const WeakPtr<Y>& other);

    WeakPtr& operator=(const SharedPtr<X>& other);

    template <class Y,
              class = std::enable_if_t<std::is_convertible_v<Y*, X*> &&
                                       !std::is_same_v<Y, void>>>
    WeakPtr& operator=(const SharedPtr<Y>& other);

    WeakPtr& operator=(WeakPtr&& other);

    template <class Y,
              class = std::enable_if_t<std::is_convertible_v<Y*, X*> &&
                                       !std::is_same_v<Y, void>>>
    WeakPtr& operator=(WeakPtr<Y>&& other);

    ~WeakPtr();

    void Reset();

    void Swap(WeakPtr& other);

    void ReleaseWeakCount();

    void Release();

    size_t UseCount() const;

    bool Expired() const;

    SharedPtr<X> Lock() const;

    template <typename T>
    friend class SharedPtr;
};

template <>
class WeakPtr<void> {
public:
    void* ptr_;
    ControlBlockBase* cntrl_;

    WeakPtr() noexcept;

    WeakPtr(const WeakPtr& other);

    template <class Y>
    WeakPtr(const WeakPtr<Y>& other);

    WeakPtr(WeakPtr&& other) noexcept;

    template <class Y>
    WeakPtr(WeakPtr<Y>&& other) noexcept;

    template <class Y>
    WeakPtr(const SharedPtr<Y>& other);

    WeakPtr& operator=(const WeakPtr& other);

    template <class Y>
    WeakPtr& operator=(const WeakPtr<Y>& other);

    WeakPtr& operator=(const SharedPtr<void>& other);

    template <class Y>
    WeakPtr& operator=(const SharedPtr<Y>& other);

    WeakPtr& operator=(WeakPtr&& other);

    template <class Y>
    WeakPtr& operator=(WeakPtr<Y>&& other);

    ~WeakPtr();

    void Reset();

    void Swap(WeakPtr& other);

    void ReleaseWeakCount();

    void Release();

    size_t UseCount() const;

    bool Expired() const;

    SharedPtr<void> Lock() const;

    template <typename T>
    friend class SharedPtr;
};

class ESFTBase;

template <typename X>
class EnableSharedFromThis;

