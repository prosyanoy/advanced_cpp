#pragma once

#include <vector>
#include <thread>

/*
template <class RandomAccessIterator, class T, class Func>
T Reduce(RandomAccessIterator first, RandomAccessIterator last, const T& initial_value, Func func) {
    auto cur_value(initial_value);
    while (first != last) {
        cur_value = func(cur_value, *first++);
    }
    return cur_value;
}
*/

template <class RandomAccessIterator, class T, class Func>
T Reduce(RandomAccessIterator first, RandomAccessIterator last, const T& initial_value, Func func) {
    if (first == last) {
        return initial_value;
    }

    size_t num_threads = std::thread::hardware_concurrency();
    size_t dist = std::distance(first, last);
    size_t ops = (dist + num_threads - 1) / num_threads;

    T result(initial_value);
    std::mutex mtx;
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads && i * ops < dist; ++i) {
        threads.emplace_back([&func, &mtx, &result, i, ops, first, last] {
            auto begin = first + i * ops;
            T cur_value = *begin;
            for (; begin != last - 1 && begin != first + (i + 1) * ops - 1;) {
                cur_value = func(cur_value, *(++begin));
            }
            mtx.lock();
            result = func(result, cur_value);
            mtx.unlock();
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    return result;
}