#pragma once

#include <cstddef>
#include <cstring>
#include <cassert>
#include <array>
#include <span>
#include <memory>

namespace util {

template <typename T>
class HeapArray {
    std::unique_ptr<T[]> ptr;
    std::size_t len = 0;

public:
    HeapArray() = default;
    explicit HeapArray(std::size_t s) { reset(s); }

    T & operator[](std::size_t pos) { return ptr[pos]; }
    T *data()                       { return ptr.get(); }
    T *begin() const                { return ptr.get(); }
    T *end() const                  { return ptr.get() + len; }
    std::size_t size() const        { return len; }

    void reset(std::size_t s)
    {
        ptr = std::make_unique<T[]>(s);
        len = s;
    }
};

template <typename T, std::size_t Width, std::size_t Height>
class Array2D {
    static_assert(Width != 0 && Height != 0, "Can't define an Array2D with 0 width or height");
    T arr[Width*Height];

public:
    constexpr std::span<T>       operator[](std::size_t pos)       { return std::span{ arr+pos*Width, Width }; }
    constexpr std::span<const T> operator[](std::size_t pos) const { return std::span{ arr+pos*Width, Width }; }

    constexpr T *data() { return arr; }
    constexpr const T *data() const { return arr; }
    constexpr std::size_t width()  const { return Width; }
    constexpr std::size_t height() const { return Height; }
};

template <typename T, unsigned N>
class StaticVector {
    T arr[N];
    T *cur = arr;

public:
    constexpr StaticVector() = default;

    constexpr       T & operator[](std::size_t pos)       { return arr[pos]; }
    constexpr const T & operator[](std::size_t pos) const { return arr[pos]; }

    constexpr       T *data()        noexcept { return arr; }
    constexpr const T *data()  const noexcept { return arr; }

    using iterator = T *;

    constexpr       iterator begin()       noexcept { return arr; }
    // constexpr const iterator begin() const noexcept { return arr; }
    constexpr       iterator end()         noexcept { return cur; }
    // constexpr const iterator end()   const noexcept { return cur; }

    [[nodiscard]] constexpr bool empty() const noexcept { return cur == arr; }
    constexpr std::size_t size() const noexcept { return cur - arr; }

    constexpr void clear() noexcept          { cur = arr; }
    constexpr void push_back(const T &value) { *cur++ = value; }
    constexpr void push_back(T &&value)      { *cur++ = std::move(value); }
    constexpr void pop_back()                { cur--; }

    template <typename... Args>
    constexpr T & emplace_back(Args&&... args)
    {
        T *ptr = new (cur) T(args...);
        cur++;
        return *ptr;
    }
};

} // namespace util
