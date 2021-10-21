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
    T arr[Width*Height];

public:
    std::span<T> operator[](std::size_t pos)
    {
        return std::span{ arr+pos*Width, Width };
    }

    T *data() { return arr; }
    const T *data() const { return arr; }
    std::size_t width()  const { return Width; }
    std::size_t height() const { return Height; }
};

} // namespace Util
