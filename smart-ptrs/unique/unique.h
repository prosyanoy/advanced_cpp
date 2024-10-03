#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <type_traits>
#include <cassert>

template <class T>
struct Slug {
    Slug() = default;

    // Template copy constructor
    template <class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
    Slug(const Slug<U>&) noexcept {
    }

    // Template move constructor
    template <class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
    Slug(Slug<U>&&) noexcept {
    }

    // Template copy assignment operator
    template <class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
    Slug& operator=(const Slug<U>&) noexcept {
        return *this;
    }

    // Template move assignment operator
    template <class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
    Slug& operator=(Slug<U>&&) noexcept {
        return *this;
    }

    void operator()(T* ptr) const noexcept {
        delete ptr;
    }
};

template <class T>
struct Slug<T[]> {
    Slug() = default;

    // Template copy constructor
    template <class U, std::enable_if_t<std::is_convertible_v<U (*)[], T (*)[]>, int> = 0>
    Slug(const Slug<U>&) noexcept {
    }

    // Template move constructor
    template <class U, std::enable_if_t<std::is_convertible_v<U (*)[], T (*)[]>, int> = 0>
    Slug(Slug<U>&&) noexcept {
    }

    // Template copy assignment operator
    template <class U, std::enable_if_t<std::is_convertible_v<U (*)[], T (*)[]>, int> = 0>
    Slug& operator=(const Slug<U>&) noexcept {
        return *this;
    }

    // Template move assignment operator
    template <class U, std::enable_if_t<std::is_convertible_v<U (*)[], T (*)[]>, int> = 0>
    Slug& operator=(Slug<U>&&) noexcept {
        return *this;
    }

    template <class U>
    void operator()(U* ptr) const noexcept {
        static_assert(std::is_convertible<U(*)[], T(*)[]>::value,
                      "U(*)[] must be convertible to T(*)[]");
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
    CompressedPair<T*, Deleter> data_;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter()) {
    }

    UniquePtr(T* ptr, Deleter&& deleter) : data_(ptr, std::forward<Deleter>(deleter)) {
    }
    UniquePtr(T* ptr, const Deleter& deleter) : data_(ptr, deleter) {
    }

    template <class U, class D,
              class = std::enable_if_t<(std::is_base_of_v<T, U> || std::is_base_of_v<U, T> ||
                                        std::is_same_v<T, U>) &&
                                       !std::is_array<U>::value>,
              class = std::enable_if_t<
                  (std::is_reference<Deleter>::value && std::is_same<Deleter, D>::value) ||
                  (!std::is_reference<Deleter>::value && std::is_convertible<D, Deleter>::value)>>
    UniquePtr(UniquePtr<U, D>&& other) noexcept
        : data_(other.Release(), std::move(other.GetDeleter())) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    template <class U, class D,
              class = std::enable_if_t<(std::is_base_of_v<T, U> || std::is_base_of_v<U, T> ||
                                        std::is_same_v<T, U>) &&
                                       !std::is_array<U>::value>,
              class = std::enable_if_t<std::is_assignable<Deleter&, D&&>::value>>
    UniquePtr& operator=(UniquePtr<U, D>&& other) noexcept {
        if (Get() != other.Get()) {
            Reset(other.Release());
            data_.GetSecond() = std::forward<D>(other.GetDeleter());
        }
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        data_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* temp = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return temp;
    }

    void Reset(T* ptr = nullptr) {
        T* old_ptr = data_.GetFirst();
        data_.GetFirst() = ptr;
        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(data_, other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() {
        return data_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }
    explicit operator bool() const {
        return data_.GetFirst();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator*() const {
        return *data_.GetFirst();
    }
    T* operator->() const {
        return data_.GetFirst();
    }
    // Запрет копирования
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;
};

// Void specialization
template <typename Deleter>
class UniquePtr<void, Deleter> {
    CompressedPair<void*, Deleter> data_;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(void* ptr = nullptr) : data_(ptr, Deleter()) {
    }

    UniquePtr(void* ptr, const Deleter& deleter) : data_(ptr, deleter) {
    }
    UniquePtr(void* ptr, Deleter&& deleter) : data_(ptr, std::forward<Deleter>(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept : data_(other.Release(), std::move(other.GetDeleter())) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (Get() != other.Get()) {
            Reset();
            data_.GetFirst() = other.Release();
            data_.GetSecond() = std::move(other.GetDeleter());
            other.Reset();
        }
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        data_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void* Release() {
        void* temp = std::move(data_.GetFirst());
        data_.GetFirst() = nullptr;
        return temp;
    }
    void Reset(void* ptr = nullptr) {
        void* old_ptr = data_.GetFirst();
        data_.GetFirst() = ptr;  // Update the pointer before deletion
        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(data_, other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    void* Get() const {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() {
        return data_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }
    explicit operator bool() const {
        return data_.GetFirst();
    }
    // Запрет копирования
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
    CompressedPair<T*, Deleter> data_;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter()) {
    }

    UniquePtr(T* ptr, const Deleter& deleter) : data_(ptr, deleter) {
    }
    UniquePtr(T* ptr, Deleter&& deleter) : data_(ptr, std::forward<Deleter>(deleter)) {
    }

    template <class U, class D,
              class = std::enable_if_t<std::is_convertible<U (*)[], T (*)[]>::value>,
              class = std::enable_if_t<
                  (std::is_reference<Deleter>::value && std::is_same<Deleter, D>::value) ||
                  (!std::is_reference<Deleter>::value && std::is_constructible_v<Deleter, D&&>)>>
    UniquePtr(UniquePtr<U[], D>&& other) noexcept
        : data_(other.Release(), std::move(other.GetDeleter())) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <class U, class D,
              class = std::enable_if_t<std::is_convertible<U (*)[], T (*)[]>::value>,
              class = std::enable_if_t<std::is_assignable_v<Deleter&, D&&>>>
    UniquePtr& operator=(UniquePtr<U[], D>&& other) noexcept {
        if (Get() != other.Get()) {
            Reset();
            data_.GetFirst() = other.Release();
            data_.GetSecond() = std::move(other.GetDeleter());
            other.Reset();
        }
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        data_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* temp = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return temp;
    }
    void Reset(T* ptr = nullptr) {
        T* old_ptr = data_.GetFirst();
        data_.GetFirst() = ptr;
        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(data_, other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T& operator[](std::size_t index) const {
        assert(data_.GetFirst() != nullptr);
        return data_.GetFirst()[index];
    }

    T* Get() const {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() {
        return data_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }
    explicit operator bool() const {
        return data_.GetFirst();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator*() const {
        return *data_.GetFirst();
    }
    T* operator->() const {
        return data_.GetFirst();
    }
    // Запрет копирования
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;
};

// Объявляем специализацию std::swap для UniquePtr
namespace std {
template <typename T, typename D>
void swap(UniquePtr<T, D>& lhs, UniquePtr<T, D>& rhs) noexcept {
    lhs.Swap(rhs);  // Вызов пользовательского метода swap
}

}