#pragma once

#include <vector>

template <class Functor>
class ReverseBinaryFunctor {
public:
    explicit ReverseBinaryFunctor(Functor functor) : functor_(functor) {
    }

    template <typename T>
    bool operator()(const T& a, const T& b) const {
        return functor_(b, a);
    }

private:
    Functor functor_;
};

template <class Functor>
class ReverseUnaryFunctor {
public:
    explicit ReverseUnaryFunctor(Functor functor) : functor_(functor) {
    }

    template <typename T>
    bool operator()(const T& a) const {
        return !functor_(a);
    }

private:
    Functor functor_;
};

template <class Functor>
ReverseUnaryFunctor<Functor> MakeReverseUnaryFunctor(Functor functor) {
    return ReverseUnaryFunctor<Functor>(functor);
}

template <class Functor>
ReverseBinaryFunctor<Functor> MakeReverseBinaryFunctor(Functor functor) {
    return ReverseBinaryFunctor<Functor>(functor);
}
