#pragma once

#include <string>
#include <cstring>
#include <algorithm>

class StringView {
    const char* sv;
    size_t sv_size;

public:
    StringView(const std::string& s, size_t pos = 0, size_t size = std::string::npos) {
        sv = s.c_str() + pos;
        sv_size = std::min(size, s.size() - pos);
    }

    StringView(const char* s) : sv(s) {
        sv_size = std::strlen(s);
    }

    StringView(const char* s, size_t size) : sv(s) {
        sv_size = size;
    }

    char operator[](size_t index) const {
        return *(sv + index);
    }

    size_t Size() const {
        return sv_size;
    }
};
