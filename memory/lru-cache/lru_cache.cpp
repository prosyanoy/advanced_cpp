#include "lru_cache.h"

LruCache::LruCache(size_t max_size) : size_(max_size) {
}

void LruCache::Set(const std::string& key, const std::string& value) {
    auto it = iters_map_.find(key);
    if (it != iters_map_.end()) {
        values_list_.erase(it->second);
        values_list_.push_back(std::make_pair(key, value));
        it->second = --values_list_.end();

    } else if (values_list_.size() == size_) {
        iters_map_.erase(values_list_.front().first);
        values_list_.pop_front();
        values_list_.push_back(std::make_pair(key, value));
        iters_map_[key] = --values_list_.end();
    } else {
        values_list_.push_back(std::make_pair(key, value));
        iters_map_[key] = --values_list_.end();
    }
}

bool LruCache::Get(const std::string& key, std::string* value) {
    auto it = iters_map_.find(key);
    if (it != iters_map_.end()) {
        *value = (*it->second).second;
        values_list_.erase(it->second);
        values_list_.push_back(std::make_pair(key, *value));
        it->second = --values_list_.end();
        return true;
    }
    return false;
}
