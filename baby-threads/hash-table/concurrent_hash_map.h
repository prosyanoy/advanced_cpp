#pragma once

#include <unordered_map>
#include <mutex>
#include <functional>
#include <list>
#include <stdexcept>
#include <atomic>
#include <vector>

template <class K, class V, class Hash = std::hash<K>>
class ConcurrentHashMap {
public:
    ConcurrentHashMap(const Hash& hasher = Hash()) : ConcurrentHashMap(kUndefinedSize, hasher) {
    }

    explicit ConcurrentHashMap(int expected_size, const Hash& hasher = Hash())
        : ConcurrentHashMap(expected_size, kDefaultConcurrencyLevel, hasher) {
    }

    ConcurrentHashMap(int expected_size, int expected_threads_count, const Hash& hasher = Hash())
        : size_(0), hasher_(hasher) {
        if (expected_size != kUndefinedSize) {
            bucket_size_ = 4 * expected_size;
        } else {
            bucket_size_ = expected_threads_count * 100;
        }
        lock_size_ = kDefaultConcurrencyLevel * 16;
        locks_ = static_cast<std::vector<std::mutex>>(lock_size_);
        table_ = std::vector<std::list<Pair>>(bucket_size_);
    }

    bool Insert(const K& key, const V& value) {
        size_t idx_lock = hasher_(key) % lock_size_;
        {
            std::lock_guard<std::mutex> lock(locks_[idx_lock]);

            // Lock rehash_mutex_ after acquiring locks_[idx_lock]
            std::lock_guard<std::mutex> rehash_lock(rehash_mutex_);

            if (size_.load(std::memory_order_relaxed) > table_.size() * kMaxLoadFactor) {
                ReHash(); // Rehashing with locks held in correct order
            }

            size_t h = hasher_(key) % bucket_size_;
            auto& l = table_[h];
            for (const auto& pair : l) {
                if (pair.key == key) {
                    return false;
                }
            }
            l.emplace_back(key, value);
            size_.fetch_add(1, std::memory_order_relaxed);
            return true;
        }
    }

    bool Erase(const K& key) {
        size_t idx_lock = hasher_(key) % lock_size_;
        std::lock_guard<std::mutex> lock(locks_[idx_lock]);

        size_t h = hasher_(key) % bucket_size_;
        auto& l = table_[h];
        auto it = std::find_if(l.begin(), l.end(), [&](const Pair& p) { return p.key == key; });
        if (it != l.end()) {
            l.erase(it);
            size_.fetch_sub(1, std::memory_order_relaxed);
            return true;
        }
        return false;
    }

    void Clear() {
        // First, lock all locks in locks_
        for (auto& lock : locks_) {
            lock.lock();
        }

        // Then, lock rehash_mutex_
        std::lock_guard<std::mutex> rehash_lock(rehash_mutex_);

        table_.clear();
        table_.resize(bucket_size_);
        size_ = 0;

        // Unlock locks in reverse order
        for (auto it = locks_.rbegin(); it != locks_.rend(); ++it) {
            it->unlock();
        }
    }

    std::pair<bool, V> Find(const K& key) const {
        size_t idx_lock = hasher_(key) % lock_size_;
        std::lock_guard<std::mutex> lock(locks_[idx_lock]);

        // Lock rehash_mutex_ after acquiring locks_[idx_lock]
        std::lock_guard<std::mutex> rehash_lock(rehash_mutex_);

        size_t h = hasher_(key) % bucket_size_;
        const auto& l = table_[h];
        for (const auto& pair : l) {
            if (pair.key == key) {
                return std::make_pair(true, pair.value);
            }
        }
        return std::make_pair(false, V());
    }

    const V At(const K& key) const {
        size_t idx_lock = hasher_(key) % lock_size_;
        std::lock_guard<std::mutex> lock(locks_[idx_lock]);

        std::lock_guard<std::mutex> rehash_lock(rehash_mutex_);

        size_t h = hasher_(key) % bucket_size_;
        const auto& l = table_[h];
        for (const auto& pair : l) {
            if (pair.key == key) {
                return pair.value;
            }
        }
        throw std::out_of_range("No such key");
    }

    size_t Size() const {
        return size_.load(std::memory_order_relaxed);
    }

    static const int kDefaultConcurrencyLevel;
    static const int kUndefinedSize;
    static const int kMaxLoadFactor;

private:
    struct Pair {
        K key;
        V value;
    };

    void ReHash() {
        // First, lock all locks in locks_
        std::vector<std::unique_lock<std::mutex>> lock_guards;
        lock_guards.reserve(locks_.size());
        for (auto& lock : locks_) {
            lock_guards.emplace_back(lock);
        }

        // Then, lock rehash_mutex_
        std::lock_guard<std::mutex> rehash_lock(rehash_mutex_);

        bucket_size_ *= 2;
        std::vector<std::list<Pair>> new_table(bucket_size_);

        for (size_t i = 0; i < table_.size(); ++i) {
            auto& old_list = table_[i];
            while (!old_list.empty()) {
                auto it = old_list.begin();
                size_t h = hasher_(it->key) % bucket_size_;
                new_table[h].splice(new_table[h].end(), old_list, it);
            }
        }
        table_.swap(new_table);
    }

    std::vector<std::list<Pair>> table_;
    mutable std::vector<std::mutex> locks_;
    mutable std::mutex rehash_mutex_;
    size_t lock_size_;
    size_t bucket_size_;
    std::atomic<size_t> size_;
    Hash hasher_;
};

template <class K, class V, class Hash>
const int ConcurrentHashMap<K, V, Hash>::kDefaultConcurrencyLevel = 64; // Increased

template <class K, class V, class Hash>
const int ConcurrentHashMap<K, V, Hash>::kUndefinedSize = -1;

template <class K, class V, class Hash>
const int ConcurrentHashMap<K, V, Hash>::kMaxLoadFactor = 4; // New constant
