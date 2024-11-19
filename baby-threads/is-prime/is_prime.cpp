#include "is_prime.h"
#include <atomic>
#include <thread>
#include <vector>
#include <cmath>
#include <algorithm>

bool IsPrime(uint64_t x) {
    if (x <= 1) {
        return false;
    }
    uint64_t num_threads = std::thread::hardware_concurrency();
    uint64_t root = static_cast<uint64_t>(std::sqrt(x));
    auto bound = std::min(root + 6, x);

    uint64_t step = (bound - 2) / num_threads + 1;

    std::atomic<bool> flag(true);
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads && (2 + i * step) < bound; ++i) {
        threads.emplace_back([&flag, step, i, x, bound] {
            uint64_t start = 2 + i * step;
            uint64_t end = std::min(start + step, bound);
            for (uint64_t k = start; k < end && flag.load(); ++k) {
                if (x % k == 0) {
                    flag.store(false);
                    return;
                }
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    return flag.load();
}