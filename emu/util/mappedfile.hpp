#pragma once

#include <span>
#include <optional>
#include <string>
#include <string_view>
#include <emu/util/unsigned.hpp>

namespace Util {

class MappedFile {
    uint8 *ptr = nullptr;
    std::size_t len = 0;
    std::string name;

    MappedFile(uint8 *p, std::size_t s, std::string_view n)
        : ptr(p), len(s), name(n)
    { }

public:
    ~MappedFile();

    MappedFile(const MappedFile &) = delete;
    MappedFile(MappedFile &&m) { operator=(std::move(m)); }
    MappedFile & operator=(const MappedFile &) = delete;
    MappedFile & operator=(MappedFile &&m)
    {
        std::swap(ptr, m.ptr);
        std::swap(len, m.len);
        std::swap(name, m.name);
        return *this;
    }

    static std::optional<MappedFile> open(std::string_view pathname);

    uint8 operator[](std::size_t index) { return ptr[index]; }
    uint8 *begin() const                { return ptr; }
    uint8 *end() const                  { return ptr + len; }
    uint8 *data() const                 { return ptr; }
    std::size_t size() const            { return len; }
    std::string filename() const        { return name; }
    std::span<uint8> slice(std::size_t start, std::size_t len) { return {ptr+start, len}; }
    std::span<uint8> contents()                                { return {ptr, len}; }
};

} // namespace Util
