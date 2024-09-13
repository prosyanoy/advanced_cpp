#pragma once

#include <string>
#include <cstring>
#include <algorithm>

class StringView {
    const char* sv_;
    size_t sv_size_;

public:
    StringView(const std::string& s, size_t pos = 0, size_t size = std::string::npos) {
        sv_ = s.c_str() + pos;
        sv_size_ = std::min(size, s.size() - pos);
    }

    StringView(const char* s) : sv_(s) {
        sv_size_ = std::strlen(s);
    }

    StringView(const char* s, size_t size) : sv_(s) {
        sv_size_ = size;
    }

    char operator[](size_t index) const {
        return *(sv_ + index);
    }

    size_t Size() const {
        return sv_size_;
    }
};
