#pragma once

#include <optional>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <functional>
#include <string>
#include "utf8.h"

using std::optional;
namespace fs = std::filesystem;

struct GrepOptions {
    optional<size_t> look_ahead_length;
    size_t max_matches_per_line;

    GrepOptions() {
        max_matches_per_line = 10;
    }

    GrepOptions(size_t look_ahead_length) : GrepOptions() {
        this->look_ahead_length = look_ahead_length;
    }

    GrepOptions(optional<size_t> look_ahead_length, size_t max_matches_per_line) {
        this->look_ahead_length = look_ahead_length;
        this->max_matches_per_line = max_matches_per_line;
    }
};

template <class Visitor>
void SearchFile(const std::string& path, const std::string& pattern, Visitor& visitor, const GrepOptions& options) {
    std::ifstream file(path);
    if (file.is_open()) {
        std::string line;
        size_t line_count = 1;
        while (std::getline(file, line)) {
            std::string_view line_view(line);
            if (!utf8::is_valid(line_view.begin(), line_view.end())) {
                visitor.OnError("Invalid UTF-8 encoding in file");
                return;
            }
            std::vector<uint32_t> line_codepoints;
            utf8::utf8to32(line.begin(), line.end(), std::back_inserter(line_codepoints));

            std::vector<uint32_t> pattern_codepoints;
            utf8::utf8to32(pattern.begin(), pattern.end(), std::back_inserter(pattern_codepoints));

            auto searcher = std::boyer_moore_searcher(
                pattern_codepoints.begin(), pattern_codepoints.end());

            size_t n = options.max_matches_per_line;
            optional<size_t> look_ahead = options.look_ahead_length;
            size_t count = 0;

            auto it = line_codepoints.begin();

            while (count < n) {
                it = std::search(it, line_codepoints.end(), searcher);

                if (it == line_codepoints.end()) {
                    break;
                }

                size_t column = it - line_codepoints.begin();

                if (look_ahead.has_value()) {
                    size_t m = look_ahead.value();

                    auto start = it + pattern_codepoints.size();
                    auto end = (start + m < line_codepoints.end()) ? (start + m) : line_codepoints.end();

                    std::string next_chars;
                    utf8::utf32to8(start, end, std::back_inserter(next_chars));

                    visitor.OnMatch(path, line_count, column + 1, next_chars);
                } else {
                    visitor.OnMatch(path, line_count, column + 1, std::nullopt);
                }

                ++it;
                ++count;
            }
            ++line_count;
        }
        file.close();
    } else {
        visitor.OnError("File is closed");
    }
}

template <class Visitor>
void IterateDirectory(const std::string& path, const std::string& pattern, Visitor& visitor,
                      const GrepOptions& options) {
    for (const auto& entry : fs::directory_iterator(path)) {
        const auto& p = entry.path();
        if (fs::is_symlink(p)) {
            continue;
        }
        if (fs::is_directory(p)) {
            IterateDirectory(p, pattern, visitor, options);
        } else {
            SearchFile(p, pattern, visitor, options);
        }
    }
}

template <class Visitor>
void Grep(const std::string& path, const std::string& pattern, Visitor visitor,
          const GrepOptions& options) {
    if (fs::exists(path)) {
        if (fs::is_symlink(path)) {
            return;
        }
        if (fs::is_directory(path)) {
            IterateDirectory(path, pattern, visitor, options);
        } else if (fs::is_regular_file(path)) {
            SearchFile(path, pattern, visitor, options);
        }
    } else {
        std::cout << "Path is wrong" << std::endl;
    }
}
