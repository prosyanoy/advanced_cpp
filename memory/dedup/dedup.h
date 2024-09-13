#pragma once

#include <memory>
#include <vector>
#include <string>

using std::string;

std::vector<std::unique_ptr<string>> Duplicate(const std::vector<std::shared_ptr<string>>& items) {
    std::vector<std::unique_ptr<string>> out(items.size());
    for (size_t i = 0; i < items.size(); i++) {
        out[i] = std::make_unique<string>(*items[i]);
    }
    return out;
}

std::vector<std::shared_ptr<string>> DeDuplicate(const std::vector<std::unique_ptr<string>>& items) {
    std::vector<std::shared_ptr<string>> out(items.size());
    for (size_t i = 0; i < items.size(); i++) {
        string s = *items[i];
        size_t j = 0;
        for (; j < i; j++) {
            if (*out[j] == s) {
                out[i] = out[j];
                break;
            }
        }
        if (j == i) {
            out[i] = std::make_shared<string>(std::move(s));
        }
    }

    return out;
}
