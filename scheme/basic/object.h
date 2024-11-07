#pragma once

#include <memory>
#include <string>
#include <error.h>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
};

class Symbol : public Object {
    std::string name_;

public:
    Symbol(std::string name);
    const std::string& GetName() const;
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) = 0;
};

class Boolean : public Object {
    bool boolean_;

public:
    Boolean(bool);
    bool GetBoolean();
};

class RealBoolean : public Boolean {
public:
    RealBoolean(bool b);
};

class BooleanTrue : public RealBoolean {
public:
    BooleanTrue();
};

class BooleanFalse : public RealBoolean {
public:
    BooleanFalse();
};

class Number : public Boolean {
    int value_;

public:
    Number(int val);
    int GetValue() const;
};

class InvalidSymbol : public Symbol {
public:
    InvalidSymbol(std::string name);
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class Quote : public Symbol {
    bool is_symbol_;

public:
    Quote(bool);
    bool IsSymbol();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class IntegerPredicate : public Symbol {
public:
    IntegerPredicate();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object> obj, bool) override;
};

class IntegerEqual : public Symbol {
public:
    IntegerEqual();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object> obj, bool) override;
};

class IntegerMore : public Symbol {
public:
    IntegerMore();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class IntegerLess : public Symbol {
public:
    IntegerLess();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class IntegerMoreEqual : public Symbol {
public:
    IntegerMoreEqual();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class IntegerLessEqual : public Symbol {
public:
    IntegerLessEqual();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class IntegerPlus : public Symbol {
public:
    IntegerPlus();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class IntegerMinus : public Symbol {
public:
    IntegerMinus();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class IntegerMultiply : public Symbol {
public:
    IntegerMultiply();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class IntegerDivide : public Symbol {
public:
    IntegerDivide();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class IntegerMax : public Symbol {
public:
    IntegerMax();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class IntegerMin : public Symbol {
public:
    IntegerMin();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class IntegerAbs : public Symbol {
public:
    IntegerAbs();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class BooleanPredicate : public Symbol {
public:
    BooleanPredicate();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class BooleanNot : public Symbol {
public:
    BooleanNot();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class BooleanAnd : public Symbol {
public:
    BooleanAnd();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class BooleanOr : public Symbol {
public:
    BooleanOr();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class PairPredicate : public Symbol {
public:
    PairPredicate();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class NullPredicate : public Symbol {
public:
    NullPredicate();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class ListPredicate : public Symbol {
public:
    ListPredicate();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class Cons : public Symbol {
public:
    Cons();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class Car : public Symbol {
public:
    Car();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class Cdr : public Symbol {
public:
    Cdr();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class List : public Symbol {
public:
    List();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class ListRef : public Symbol {
public:
    ListRef();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class ListTail : public Symbol {
public:
    ListTail();
    virtual std::shared_ptr<Object> Do(std::shared_ptr<Object>, bool) override;
};

class Cell : public Object {
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;

public:
    Cell();

    void SetFirst(std::shared_ptr<Object> f);
    void SetSecond(std::shared_ptr<Object> s);

    std::shared_ptr<Object> GetFirst() const;
    std::shared_ptr<Object> GetSecond() const;
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

std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> obj);
