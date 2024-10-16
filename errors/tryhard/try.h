#pragma once

#include <exception>
#include <stdexcept>
#include <cstring>

template <class T>
class Try {
    T value_;
    bool IsEmpty_;
    std::exception_ptr ex_;

public:
    Try() : value_(), IsEmpty_(true), ex_(nullptr) {
    }
    Try(T value) : value_(value), IsEmpty_(false), ex_(nullptr) {
    }
    Try(const std::exception& ex) : value_(), IsEmpty_(true) {
        ex_ = std::make_exception_ptr(std::runtime_error(ex.what()));
    }
    Try(std::exception_ptr ex, bool) : value_(), IsEmpty_(true), ex_(ex) {
    }
    Try(const char* ex)
        : value_(), IsEmpty_(true), ex_(std::make_exception_ptr(std::runtime_error(ex))) {
    }
    Try(int ex, bool)
        : value_(),
          IsEmpty_(true),
          ex_(std::make_exception_ptr(std::runtime_error(std::strerror(ex)))) {
    }

    const T& Value() {
        if (ex_) {
            std::rethrow_exception(ex_);
        } else {
            if (IsEmpty_) {
                throw std::runtime_error("Object is empty");
            } else {
                return value_;
            }
        }
    }

    void Throw() {
        if (ex_) {
            std::rethrow_exception(ex_);
        } else {
            throw std::runtime_error("No exception");
        }
    }

    bool IsFailed() {
        return ex_ != nullptr;
    }
};

template <>
class Try<void> {
    std::exception_ptr ex_;

public:
    Try() : ex_(nullptr) {
    }
    Try(const std::exception& ex) {
        ex_ = std::make_exception_ptr(std::runtime_error(ex.what()));
    }
    Try(std::exception_ptr ex, bool) : ex_(ex) {
    }
    Try(const char* ex) : ex_(std::make_exception_ptr(std::runtime_error(ex))) {
    }
    Try(int ex, bool) : ex_(std::make_exception_ptr(std::runtime_error(std::strerror(ex)))) {
    }
    void Throw() {
        if (ex_) {
            std::rethrow_exception(ex_);
        } else {
            throw std::runtime_error("No exception");
        }
    }
    bool IsFailed() {
        return ex_ != nullptr;
    }
};

template <class Function, class... Args>
auto TryRun(Function func, Args... args) {
    using ReturnType = decltype(func(args...));
    try {
        func(args...);
        if constexpr (std::is_void_v<ReturnType>) {
            return Try<void>();
        } else {
            return Try<ReturnType>(func(args...));
        }
    } catch (const std::exception& ex) {
        std::exception_ptr eptr = std::current_exception();
        return Try<ReturnType>(eptr, true);
    } catch (const char* ex) {
        return Try<ReturnType>(ex);
    } catch (int ex) {
        return Try<ReturnType>(ex, true);
    } catch (...) {
        return Try<ReturnType>("Unknown exception");
    }
}