#ifndef CIRCULARBUFFER_HPP_INCLUDED
#define CIRCULARBUFFER_HPP_INCLUDED

#include <cstddef>
#include <iterator>

template <typename T, std::size_t Size>
class CircularBuffer {
    T arr[Size];
    // head points to the newest elem, tail points to the oldest elem
    std::size_t head = 0;
    std::size_t tail = 0;

public:
    void push(T newelem)
    {
        std::size_t next = (head + 1) % Size;
        if (next == tail) // full
            tail = (tail + 1) % Size;
        arr[head] = newelem;
        head = next;
    }

    T pop()
    {
        if (head == tail) // empty
            return T();
        std::size_t next = (tail + 1) % Size;
        T &toret = arr[tail];
        tail = next;
        return toret;
    }

    std::size_t size() const
    {
        int res = head - tail;
        return res >= 0 ? res : Size + res + 1;
    }

    T & operator[](const std::size_t index)
    {
        // oldest -> newest
        // arr[(tail + index) % Size]; }
        // newest -> oldest
        return arr[(head - 1 + Size - index) % Size];
    }

    bool full() const   { return head + 1 == tail; }
    bool empty() const  { return head == tail; }
    T *data() const     { return arr; }

    using value_type      = T;
    using reference       = T &;
    using const_reference = const T &;
    using pointer         = T *;
    using difference_type = std::ptrdiff_t;
    using size_type       = std::size_t;

    class iterator {
        CircularBuffer *buf;
        std::size_t index;
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using reference  = T &;
        using pointer    = T *;
        using difference_type = std::ptrdiff_t;

        iterator(CircularBuffer *c, std::size_t i) : buf(c), index(i) { }
        iterator(const iterator &i)                : buf(i.buf), index(i.index) { }
        iterator &  operator= (const iterator &i)       { buf = i.buf; index = i.index; return *this; }
        iterator &  operator++()                        { ++index; return *this; }
        iterator &  operator--()                        { --index; return *this; }
        iterator    operator++(int)                     { iterator res(*this, index); ++(*this); return res; }
        iterator    operator--(int)                     { iterator res(*this, index); --(*this); return res; }
        iterator &  operator+=(std::size_t n)           { index += n; return *this; }
        iterator &  operator-=(std::size_t n)           { index -= n; return *this; }
        iterator    operator+ (std::size_t n) const     { return iterator(buf, index+n); }
        iterator    operator- (std::size_t n) const     { return iterator(buf, index-n); }
        std::size_t operator+ (const iterator &i) const { return index + i.index; }
        std::size_t operator- (const iterator &i) const { return index - i.index; }
        bool        operator==(const iterator &i)       { return buf == i.buf && index == i.index; }
        bool        operator!=(const iterator &i)       { return buf != i.buf || index != i.index; }
        bool        operator< (const iterator &i) const { return index < i.index; }
        reference   operator*() const                   { return (*buf)[index]; }
        pointer     operator->()                        { return &buf[index]; }
    };

    iterator begin() { return iterator(this, 0); }
    iterator end()   { return iterator(this, this->size()); }
};

#endif
