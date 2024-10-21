#pragma once
#include <memory>

class Any {
    struct InnerBase {
        using Pointer = std::unique_ptr<InnerBase>;
        virtual ~InnerBase() {}
        virtual InnerBase* Clone() const = 0;
        virtual bool Empty() = 0;
    };
    template <typename T>
    struct Inner : InnerBase {
        Inner(const T& newval, bool is_empty) : value_(newval), IsEmpty_(is_empty) {
        }
        virtual InnerBase* Clone() const override {
            return new Inner(value_, IsEmpty_);
        }
        T& operator*() {
            return value_;
        }
        const T& operator*() const {
            return value_;
        }
        virtual bool Empty() override {
            return IsEmpty_;
        }
    private:
        T value_;
        bool IsEmpty_;
    };
    InnerBase::Pointer inner_;

public:
    Any() : inner_(new Inner<int>(0, true)) {
    }

    template <class T>
    Any(const T& value) : inner_(new Inner<T>(value, false)) {
    }

    template <class T>
    Any& operator=(const T& value) {
        inner_ = std::make_unique<Inner<T>>(value, false);
        return *this;
    }

    Any(const Any& rhs) : inner_(rhs.inner_->Clone()) {
    }

    Any& operator=(const Any& rhs) {
        Any tmp(rhs);
        inner_.swap(tmp.inner_);
        return *this;
    }

    bool Empty() const {
        return inner_->Empty();
    }

    void Clear() {
        Any tmp;
        inner_.swap(tmp.inner_);
    }
    void Swap(Any& rhs) {
        inner_.swap(rhs.inner_);
    }

    template <class T>
    const T& GetValue() const {
        if (auto ptr = dynamic_cast<Inner<T>*>(inner_.get())) {
            return **ptr;
        } else {
            throw std::bad_cast();
        }
    }
};
