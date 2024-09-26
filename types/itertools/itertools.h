#pragma once

#include <utility>
#include <cstdint>

template <class Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end) : begin_(begin), end_(end) {
    }

    Iterator begin() const {  // NOLINT
        return begin_;
    }

    Iterator end() const {  // NOLINT
        return end_;
    }

private:
    Iterator begin_, end_;
};

template <class FIt, class SIt, template <class, class> typename Iterator>
class IteratorZip {
public:
    typedef Iterator<FIt, SIt> iterator;
    typedef typename std::pair<typename FIt::value_type, typename SIt::value_type> value_type;

    IteratorZip(iterator begin, iterator end) : begin_(begin), end_(end) {
    }

    iterator begin() const {  // NOLINT
        return begin_;
    }

    iterator end() const {  // NOLINT
        return end_;
    }

private:
    iterator begin_, end_;
};

template <class TIt, template <class> typename Iterator>
class IteratorGroup {
public:
    typedef Iterator<TIt> iterator;
    typedef typename TIt::value_type value_type;

    IteratorGroup(iterator begin, iterator end) : begin_(begin), end_(end) {
    }

    iterator begin() const {  // NOLINT
        return begin_;
    }

    iterator end() const {  // NOLINT
        return end_;
    }

private:
    iterator begin_, end_;
};

struct RangeIt {
    typedef int64_t* iterator;
    typedef int64_t value_type;
    RangeIt(int64_t curr = 0, int64_t step = 1) : curr_(curr), step_(step) {
    }
    RangeIt operator++() {
        curr_ += step_;
        return *this;
    }

    int64_t operator*() const {
        return curr_;
    }

    friend bool operator!=(const RangeIt& a, const RangeIt& b) {
        if (a.step_ > 0) {
            return a.curr_ < b.curr_;
        } else {
            return a.curr_ > b.curr_;
        }
    };

private:
    int64_t curr_;
    int64_t step_;
};

template <typename FIt, typename SIt>
struct ZipIt {
    ZipIt(FIt first, SIt second) : first_it_(first), second_it_(second) {
    }

    ZipIt operator++() {
        ++first_it_;
        ++second_it_;
        return *this;
    }

    std::pair<typename FIt::value_type, typename SIt::value_type> operator*() const {
        return std::make_pair(*first_it_, *second_it_);
    }

    friend bool operator!=(const ZipIt& a, const ZipIt& b) {
        return (a.first_it_ != b.first_it_) && (a.second_it_ != b.second_it_);
    }

private:
    FIt first_it_;
    SIt second_it_;
};

template <typename TIt>
struct GroupIt {
    GroupIt(TIt data, TIt end) : curr_(data), end_(end) {
    }

    GroupIt operator++() {
        auto curr = *curr_;
        while (curr_ != end_ && *curr_ == curr) {
            ++curr_;
        }
        return *this;
    }

    IteratorRange<TIt> operator*() const {
        auto curr_value = *curr_;
        auto it = curr_;
        while (it != end_ && *it == curr_value) {
            ++it;
        }
        return {curr_, it};
    }

    friend bool operator!=(const GroupIt& a, const GroupIt& b) {
        return a.curr_ != b.curr_;
    }

private:
    TIt curr_;
    TIt end_;
};

IteratorRange<RangeIt> Range(int64_t from, int64_t to, int64_t step) {
    return IteratorRange<RangeIt>(RangeIt(from, step), RangeIt(to, step));
}

IteratorRange<RangeIt> Range(int64_t from, int64_t to) {
    return IteratorRange<RangeIt>(RangeIt(from), RangeIt(to));
}

IteratorRange<RangeIt> Range(int64_t to) {
    return IteratorRange<RangeIt>(RangeIt(), RangeIt(to));
}

template <typename F, typename S>
auto Zip(const F& first, const S& second) {
    using FIt = decltype(first.begin());
    using SIt = decltype(second.begin());
    using Iterator = ZipIt<FIt, SIt>;
    return IteratorZip<FIt, SIt, ZipIt>(Iterator(first.begin(), second.begin()),
                                        Iterator(first.end(), second.end()));
}

template <typename T>
auto Group(const T& data) {
    using TIt = decltype(data.begin());
    using Iterator = GroupIt<TIt>;
    return IteratorGroup<TIt, GroupIt>(Iterator(data.begin(), data.end()),
                                       Iterator(data.end(), data.end()));
}