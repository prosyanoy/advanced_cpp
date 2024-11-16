#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <fstream>

constexpr uint64_t m = 0xc6a4a7935bd1e995ULL;
constexpr uint64_t minv = 0x5f7a0ea7e59b19bdULL;
constexpr int r = 47;
size_t seed = static_cast<size_t>(0xc70f6907UL);
const char* alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";

constexpr uint64_t start_state = 11792061695843878588ULL;

bool IsInAlphabet(char c) {
    return (c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c == 95);
}

bool GenerateString(std::atomic<size_t>& counter, const uint64_t first_byte, std::atomic<bool>& done, uint64_t hash, std::vector<std::string>& thread_strings, std::mt19937& gen, std::uniform_int_distribution<size_t>& dist) {
    std::string result;
    uint64_t state;
    // Получение первых 8 символов
    while (true) {
        char p[8];
        state = start_state;
        for (size_t k = 0; k < 8; ++k) {
            size_t ind = dist(gen);
            p[k] = alphabet[ind];
        }
        uint64_t data;
        std::memcpy(&data, p, sizeof(uint64_t));
        data *= m;
        data ^= data >> r;
        data *= m;
        state ^= data;
        state *= m;
        if (state >> 56 == first_byte) {
            result.assign(p, 8);
            break;
        }
    }
    // Получение последних 7 символов
    uint64_t last7 = hash ^ state;
    size_t k = 0;
    for (; k < 7; ++k) {
        char c = last7 % 256;
        if (!IsInAlphabet(c)) {
            break;
        }
        result += c;
        last7 /= 256;
    }
    if (k == 7) {
        thread_strings.push_back(result);
        size_t prev_counter = counter.fetch_add(1, std::memory_order_relaxed) + 1;
        if (prev_counter >= 20000) {
            done.store(true, std::memory_order_relaxed);
            return true;
        }
    }
    return false;
}

void InvertMurmurHash64A(uint64_t hash) {
    hash ^= hash >> r;
    hash *= minv;
    hash ^= hash >> r;
    hash *= minv;

    uint64_t first_byte = hash >> 56;

    size_t num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    std::atomic<size_t> counter{0};
    std::atomic<bool> done{false};

    // Массив для хранения результатов от всех потоков
    std::vector<std::vector<std::string>> all_strings(num_threads);

    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([i, &counter, &done, first_byte, hash, &all_strings]{
            std::vector<std::string>& thread_strings = all_strings[i];
            std::mt19937 gen{std::random_device{}()};
            std::uniform_int_distribution<size_t> dist(0, 62);
            while (!done.load(std::memory_order_relaxed)) {
                if (GenerateString(counter, first_byte, done, hash, thread_strings, gen, dist)) {
                    break;
                }
            }
        });
    }
    for (auto& t : threads) t.join();

    // Запись результатов в файл
    std::ofstream outfile("results.txt");
    size_t total_strings = 0;
    for (const auto& thread_strings : all_strings) {
        for (const auto& str : thread_strings) {
            outfile << str << '\n';
            ++total_strings;
        }
    }
    outfile.close();
    std::cout << "Всего строк: " << total_strings << std::endl;
}

int main() {
    uint64_t hash = 69420;
    InvertMurmurHash64A(hash);
    return 0;
}
