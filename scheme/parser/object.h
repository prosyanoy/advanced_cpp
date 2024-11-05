#pragma once

#include <memory>
#include <string>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
};

class Number : public Object {
    int value_;

public:
    Number(int val) : value_(val) {
    }
    int GetValue() const {
        return value_;
    }
};

class Symbol : public Object {
    std::string name_;

public:
    Symbol(std::string name) : name_(name) {
    }
    const std::string& GetName() const {
        return name_;
    }
};

class Cell : public Object {
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;

public:
    Cell() : first_(nullptr), second_(nullptr) {
    }

    void SetFirst(std::shared_ptr<Object> f) {
        first_ = f;
    }
    void SetSecond(std::shared_ptr<Object> s) {
        second_ = s;
    }

    std::shared_ptr<Object> GetFirst() const {
        return first_;
    }
    std::shared_ptr<Object> GetSecond() const {
        return second_;
    }
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and conversion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj) != nullptr;
}
