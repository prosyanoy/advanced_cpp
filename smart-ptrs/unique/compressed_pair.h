#pragma once

#include <type_traits>
#include <utility>

template <typename T>
constexpr bool kIsFinalV = __is_final(T);

template <typename T1, typename T2>
struct FancyPairT1T2 : T1, T2 {

    FancyPairT1T2() : T1(), T2() {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_copy_constructible_v<U1> && std::is_copy_constructible_v<U2>, int> = 0>
    FancyPairT1T2(const T1& first, const T2& second) : T1(first), T2(second) {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_copy_constructible_v<U1> && std::is_move_constructible_v<U2>, int> = 0>
    FancyPairT1T2(const T1& first, T2&& second) : T1(first), T2(std::move(second)) {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_move_constructible_v<U1> && std::is_copy_constructible_v<U2>, int> = 0>
    FancyPairT1T2(T1&& first, const T2& second) : T1(std::move(first)), T2(std::move(second)) {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_move_constructible_v<U1> && std::is_move_constructible_v<U2>, int> = 0>
    FancyPairT1T2(T1&& first, T2&& second) : T1(std::move(first)), T2(std::move(second)) {
    }

    const T1& GetFirst() const {
        return *this;
    }
    T1& GetFirst() {
        return *this;
    }

    const T2& GetSecond() const {
        return *this;
    }
    T2& GetSecond() {
        return *this;
    }
};

// Оптимизированная пара, наследующаяся от T1
template <typename T1, typename T2>
struct FancyPairT1 : T1 {
    T2 second;

    FancyPairT1() : T1(), second() {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_copy_constructible_v<U1> && std::is_copy_constructible_v<U2>, int> = 0>
    FancyPairT1(const T1& first, const T2& second) : T1(first), second(second) {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_copy_constructible_v<U1> && std::is_move_constructible_v<U2>, int> = 0>
    FancyPairT1(const T1& first, T2&& second) : T1(first), second(std::move(second)) {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_move_constructible_v<U1> && std::is_copy_constructible_v<U2>, int> = 0>
    FancyPairT1(T1&& first, const T2& second) : T1(std::move(first)), second(std::move(second)) {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_move_constructible_v<U1> && std::is_move_constructible_v<U2>, int> = 0>
    FancyPairT1(T1&& first, T2&& second) : T1(std::move(first)), second(std::move(second)) {
    }

    const T1& GetFirst() const {
        return *this;
    }
    T1& GetFirst() {
        return *this;
    }

    const T2& GetSecond() const {
        return second;
    }
    T2& GetSecond() {
        return second;
    }
};

// Оптимизированная пара, наследующаяся от T2
template <typename T1, typename T2>
struct FancyPairT2 : T2 {
    T1 first;

    FancyPairT2() : first(), T2() {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_copy_constructible_v<U1> && std::is_copy_constructible_v<U2>, int> = 0>
    FancyPairT2(const T1& first, const T2& second) : first(first), T2(second) {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_copy_constructible_v<U1> && std::is_move_constructible_v<U2>, int> = 0>
    FancyPairT2(const T1& first, T2&& second) : first(first), T2(std::move(second)) {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_move_constructible_v<U1> && std::is_copy_constructible_v<U2>, int> = 0>
    FancyPairT2(T1&& first, const T2& second) : first(std::move(first)), T2(std::move(second)) {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_move_constructible_v<U1> && std::is_move_constructible_v<U2>, int> = 0>
    FancyPairT2(T1&& first, T2&& second) : T2(std::move(second)), first(std::move(first)) {
    }

    const T1& GetFirst() const {
        return first;
    }
    T1& GetFirst() {
        return first;
    }

    const T2& GetSecond() const {
        return *this;
    }
    T2& GetSecond() {
        return *this;
    }
};

// Обычная пара, если ни один из типов не является пустым
template <typename T1, typename T2>
struct NormalPair {
    T1 first;
    T2 second;

    NormalPair() : first(), second() {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_copy_constructible_v<U1> && std::is_copy_constructible_v<U2>, int> = 0>
    NormalPair(const T1& first, const T2& second) : first(first), second(second) {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_copy_constructible_v<U1> && std::is_move_constructible_v<U2>, int> = 0>
    NormalPair(const T1& first, T2&& second) : first(first), second(std::move(second)) {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_move_constructible_v<U1> && std::is_copy_constructible_v<U2>, int> = 0>
    NormalPair(T1&& first, const T2& second) : first(std::move(first)), second(std::move(second)) {
    }

    template <typename U1 = T1, typename U2 = T2,
              typename std::enable_if_t<
                  std::is_move_constructible_v<U1> && std::is_move_constructible_v<U2>, int> = 0>
    NormalPair(T1&& first, T2&& second) : first(std::move(first)), second(std::move(second)) {
    }

    const T1& GetFirst() const {
        return first;
    }
    T1& GetFirst() {
        return first;
    }

    const T2& GetSecond() const {
        return second;
    }
    T2& GetSecond() {
        return second;
    }
};

// Оптимизированная пара, выбирающая наиболее подходящую реализацию
template <typename T1, typename T2>
using CompressedPair = typename std::conditional_t<
    std::is_empty_v<T1> && std::is_empty_v<T2> && std::is_class<T1>::value &&
        std::is_class<T2>::value && !kIsFinalV<T1> && !kIsFinalV<T2> &&
        !std::is_base_of_v<T1, T2> && !std::is_base_of_v<T2, T1>,
    FancyPairT1T2<T1, T2>,
    typename std::conditional_t<
        std::is_empty_v<T1> && std::is_class<T1>::value && !kIsFinalV<T1>, FancyPairT1<T1, T2>,
        typename std::conditional_t<std::is_empty_v<T2> && std::is_class<T2>::value &&
                                        !kIsFinalV<T2>,
                                    FancyPairT2<T1, T2>, NormalPair<T1, T2> > > >;
