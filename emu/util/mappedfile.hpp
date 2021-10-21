#pragma once

#include <span>
#include <optional>
#include <string>
#include <string_view>
#include <emu/util/uint.hpp>

namespace io {

class MappedFile {
    u8 *ptr = nullptr;
    std::size_t len = 0;
    std::string name;

    MappedFile(u8 *p, std::size_t s, std::string_view n)
        : ptr(p), len(s), name(n)
    { }

public:
    ~MappedFile();

    MappedFile(MappedFile &&m) noexcept { operator=(std::move(m)); }
    MappedFile & operator=(MappedFile &&m) noexcept
    {
        std::swap(ptr, m.ptr);
        std::swap(len, m.len);
        std::swap(name, m.name);
        return *this;
    }

    static std::optional<MappedFile> open(std::string_view pathname);

    u8 operator[](std::size_t index) { return ptr[index]; }
    u8 *begin() const                { return ptr; }
    u8 *end() const                  { return ptr + len; }
    u8 *data() const                 { return ptr; }
    std::size_t size() const            { return len; }
    std::string filename() const        { return name; }
    std::span<u8> slice(std::size_t start, std::size_t length) { return { ptr + start, length}; }
};

} // namespace io