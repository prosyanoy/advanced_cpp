#pragma once

#include <string>

// Should not allow reuse and yell under sanitizers.
// Fix the interface and implementation.
// AwesomeCallback should add "awesomeness".

class OneTimeCallback {
public:
    virtual ~OneTimeCallback() = default;
    virtual std::string operator()() const&& = 0;
};

// Implement ctor, operator(), maybe something else...
class AwesomeCallback : public OneTimeCallback {
    std::string text_;

public:
    AwesomeCallback(std::string text) : text_(text) {
    }
    std::string operator()() const&& override {
        std::string s = std::move(text_) + "awesomeness";
        delete this;
        return s;
    }
};
