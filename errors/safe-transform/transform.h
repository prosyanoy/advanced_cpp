#pragma once
#include <list>

template <typename Iterator>
void Restore(std::list<Iterator> it_log, std::list<typename Iterator::value_type> value_log) {
    auto it_it = it_log.begin();
    if (value_log.begin() == value_log.end()) {
        return;
    }
    for (auto it = value_log.begin(); it != value_log.end(); ++it, ++it_it) {
        **it_it = *it;
    }
}

template <typename Iterator, typename Predicate, typename Functor>
void TransformIf(Iterator begin, Iterator end, Predicate p, Functor f) {
    std::list<Iterator> it_log;
    std::list<typename Iterator::value_type> value_log;
    for (auto it = begin; it != end; ++it) {
        bool b;
        try {
            b = p(*it);
        } catch (...) {
            Restore(it_log, value_log);
            throw;
        }
        if (b) {
            auto v = *it;
            try {
                f(v);
            } catch (...) {
                Restore(it_log, value_log);
                throw;
            }
            try {
                auto v0 = *it;
                *it = v;
                value_log.push_back(v0);
                it_log.push_back(it);
            } catch (...) {
            }
        }
    }
}
