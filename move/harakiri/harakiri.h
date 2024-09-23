#pragma once

#include <string>

// Should not allow reuse and yell under sanitizers.
// Fix the interface and implementation.
// AwesomeCallback should add "awesomeness".

class OneTimeCallback {
public:
    virtual ~OneTimeCallback() = 0;
    virtual std::string operator()() const&& = 0;
};

// Implement ctor, operator(), maybe something else...
class AwesomeCallback : public OneTimeCallback {
    std::string text_;

public:
    AwesomeCallback(std::string text) : text_(text) {
    }
    ~AwesomeCallback() {
        delete this;
    }
    std::string operator()() const&& override {
        return std::move(text_) + "awesomeness";
    }
};
