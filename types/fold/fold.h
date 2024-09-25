#pragma once

#include <vector>
#include <string>

struct Sum {
    int operator()(int a, int b) {
        return a + b;
    }
};

struct Prod {
    int operator()(int a, int b) {
        return a * b;
    }
};

struct Concat {
    std::vector<int>& operator()(std::vector<int> a, std::vector<int> b) {
        a.insert(a.end(), b.begin(), b.end());
        return a;
    }
};

template <class Iterator, class T, class BinaryOp>
T Fold(Iterator first, Iterator last, T init, BinaryOp func) {
    for (; first != last; ++first) {
        init = func(init, *first);
    }
    return init;
}

class Length {
    int* length_;
public:
    Length(int* cnt) : length_(cnt) {
    }
    template<class... Args>
    int operator()(Args&&... _) {
        return ++(*length_);
    }
};
