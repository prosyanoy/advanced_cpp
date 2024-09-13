#pragma once

#include <memory>
#include <utility>
#include <initializer_list>
#include <algorithm>

class Deque {
public:
    Deque() : buffer_(nullptr), buffer_size_(0), begin_(0, 0), end_(0, 0) {
    }

    Deque(const Deque& rhs) {
        if (!rhs.buffer_) {
            buffer_ = nullptr;
            buffer_size_ = 0;
            begin_ = std::make_pair(0, 0);
            end_ = std::make_pair(0, 0);
            return;
        }
        buffer_size_ = rhs.buffer_size_;
        buffer_ = std::make_unique<std::unique_ptr<int[]>[]>(buffer_size_);
        for (size_t i = 0; i < buffer_size_; ++i) {
            buffer_[i] = std::make_unique<int[]>(kBlockSize);
        }
        for (size_t block_ind = rhs.begin_.first; block_ind <= rhs.end_.first; ++block_ind) {
            size_t curr_ind = (block_ind == rhs.begin_.first) ? rhs.begin_.second : 0;
            size_t end_ind = (block_ind == rhs.end_.first) ? rhs.end_.second : kBlockSize;
            for (; curr_ind < end_ind; ++curr_ind) {
                buffer_[block_ind][curr_ind] = rhs.buffer_[block_ind][curr_ind];
            }
        }
        begin_ = rhs.begin_;
        end_ = rhs.end_;
    }

    Deque(Deque&& rhs) noexcept
        : buffer_(std::move(rhs.buffer_)),
          buffer_size_(rhs.buffer_size_),
          begin_(rhs.begin_),
          end_(rhs.end_) {
        rhs.buffer_size_ = 0;
        rhs.begin_ = std::make_pair(0, 0);
        rhs.end_ = std::make_pair(0, 0);
    }

    explicit Deque(size_t size) {
        if (size == 0) {
            buffer_ = nullptr;
            buffer_size_ = 0;
            begin_ = std::make_pair(0, 0);
            end_ = std::make_pair(0, 0);
            return;
        }
        buffer_size_ = size / kBlockSize + 1;
        buffer_ = std::make_unique<std::unique_ptr<int[]>[]>(buffer_size_);
        for (size_t i = 0; i < buffer_size_; ++i) {
            buffer_[i] = std::make_unique<int[]>(kBlockSize);
            std::fill(buffer_[i].get(), buffer_[i].get() + kBlockSize, int());
        }
        begin_ = std::make_pair(buffer_size_ / 2, 0);
        end_ = std::make_pair(buffer_size_ / 2 + size / kBlockSize, size % kBlockSize);
    }

    Deque(std::initializer_list<int> list) {
        size_t mem = list.size() * sizeof(int);
        buffer_size_ = 3 * (mem / 512 + 1);
        buffer_ = std::make_unique<std::unique_ptr<int[]>[]>(buffer_size_);
        begin_ = std::make_pair(buffer_size_ / 3, 0);
        end_ = std::make_pair(begin_.first + mem / 512, (mem % 512) / sizeof(int));

        size_t l = 0;
        for (size_t i = 0; i < buffer_size_; ++i) {
            buffer_[i] = std::make_unique<int[]>(kBlockSize);
        }
        for (size_t block_ind = begin_.first; block_ind <= end_.first; ++block_ind) {
            size_t curr_ind = (block_ind == begin_.first) ? begin_.second : 0;
            size_t end_ind = (block_ind == end_.first) ? end_.second : kBlockSize;
            for (; curr_ind < end_ind; ++curr_ind) {
                buffer_[block_ind][curr_ind] = *(list.begin() + l);
                ++l;
            }
        }
    }

    Deque& operator=(const Deque& rhs) {
        if (this == &rhs) {
            return *this;
        }
        Deque temp(rhs);
        Swap(temp);
        return *this;
    }

    Deque& operator=(Deque&& rhs) noexcept {
        Clear();
        buffer_ = std::move(rhs.buffer_);
        buffer_size_ = rhs.buffer_size_;
        begin_ = rhs.begin_;
        end_ = rhs.end_;
        rhs.buffer_size_ = 0;
        rhs.begin_ = std::make_pair(0, 0);
        rhs.end_ = std::make_pair(0, 0);
        return *this;
    }

    ~Deque() = default;

    void Swap(Deque& rhs) {
        std::swap(buffer_, rhs.buffer_);
        std::swap(buffer_size_, rhs.buffer_size_);
        std::swap(begin_, rhs.begin_);
        std::swap(end_, rhs.end_);
    }

    void Reserve(bool begin) {
        auto temp_buffer = std::make_unique<std::unique_ptr<int[]>[]>(buffer_size_ * 2);

        size_t i = 0;
        if (begin) {
            for (; i < buffer_size_; ++i) {
                temp_buffer[i] = std::make_unique<int[]>(kBlockSize);
            }
            begin_ = std::make_pair(begin_.first + buffer_size_, begin_.second);
            end_ = std::make_pair(end_.first + buffer_size_, end_.second);
        }

        for (size_t j = 0; j < buffer_size_; ++i, ++j) {
            temp_buffer[i] = std::move(buffer_[j]);
        }

        if (!begin) {
            for (; i < buffer_size_ * 2; ++i) {
                temp_buffer[i] = std::make_unique<int[]>(kBlockSize);
            }
        }

        buffer_ = std::move(temp_buffer);
        buffer_size_ *= 2;
    }

    void PushBack(int value) {
        if (!buffer_) {
            *this = Deque(1);
            PopBack();
        }
        if (end_.first == buffer_size_) {
            Reserve(false);
        }
        buffer_[end_.first][end_.second] = value;
        if (++end_.second == kBlockSize) {
            end_.second = 0;
            ++end_.first;
        }
    }

    void PushFront(int value) {
        if (!buffer_) {
            *this = Deque(1);
            PopFront();
        }
        if (begin_.first == 0 && begin_.second == 0) {
            Reserve(true);
        }
        if (begin_.second == 0) {
            begin_.second = kBlockSize - 1;
            --begin_.first;
        } else {
            --begin_.second;
        }
        buffer_[begin_.first][begin_.second] = value;
    }

    void PopBack() {
        if (end_.second == 0) {
            end_.second = kBlockSize - 1;
            --end_.first;
        } else {
            --end_.second;
        }
        buffer_[end_.first][end_.second] = int();
    }

    void PopFront() {
        buffer_[begin_.first][begin_.second] = int();
        if (++begin_.second == kBlockSize) {
            begin_.second = 0;
            ++begin_.first;
        }
    }

    int& operator[](size_t ind) {
        size_t block_ind = begin_.first + (begin_.second + ind) / kBlockSize;
        size_t curr_ind = (begin_.second + ind) % kBlockSize;
        return buffer_[block_ind][curr_ind];
    }

    int operator[](size_t ind) const {
        size_t block_ind = begin_.first + (begin_.second + ind) / kBlockSize;
        size_t curr_ind = (begin_.second + ind) % kBlockSize;
        return buffer_[block_ind][curr_ind];
    }

    size_t Size() const {
        return (end_.first - begin_.first) * kBlockSize + end_.second - begin_.second;
    }

    void Clear() {
        buffer_.reset();
        buffer_size_ = 0;
        begin_ = std::make_pair(0, 0);
        end_ = std::make_pair(0, 0);
    }

private:
    std::unique_ptr<std::unique_ptr<int[]>[]> buffer_;
    size_t buffer_size_;
    std::pair<size_t, size_t> begin_;
    std::pair<size_t, size_t> end_;
    static constexpr size_t kBlockSize = 512 / sizeof(int);
};
