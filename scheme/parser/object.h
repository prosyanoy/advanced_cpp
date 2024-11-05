#pragma once

#include <memory>
#include <string>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
};

class Number : public Object {
    int value;

public:
    Number(int val) : value(val) {
    }
    int GetValue() const {
        return value;
    }
};

class Symbol : public Object {
    std::string name;

public:
    Symbol(std::string name) : name(name) {
    }
    const std::string& GetName() const {
        return name;
    }
};

class Cell : public Object {
    std::shared_ptr<Object> first;
    std::shared_ptr<Object> second;

public:
    Cell() : first(nullptr), second(nullptr) {
    }

    void SetFirst(std::shared_ptr<Object> f) {
        first = f;
    }
    void SetSecond(std::shared_ptr<Object> s) {
        second = s;
    }

    std::shared_ptr<Object> GetFirst() const {
        return first;
    }
    std::shared_ptr<Object> GetSecond() const {
        return second;
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
