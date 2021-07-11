#pragma once

#include <span>
#include <optional>
#include <string_view>
#include <emu/util/unsigned.hpp>

namespace Util {

class MappedFile {
    uint8 *ptr = nullptr;
    std::size_t len = 0;

    MappedFile(uint8 *p, std::size_t s)
        : ptr(p), len(s)
    { }

public:
    ~MappedFile();

    MappedFile(const MappedFile &) = delete;
    MappedFile(MappedFile &&) = default;
    MappedFile & operator=(const MappedFile &) = delete;
    MappedFile & operator=(MappedFile &&) = default;

    static std::optional<MappedFile> open(std::string_view pathname);

    uint8 operator[](std::size_t index) { return ptr[index]; }
    uint8 *begin() const                { return ptr; }
    uint8 *end() const                  { return ptr + len; }
    uint8 *data() const                 { return ptr; }
    std::size_t size() const            { return len; }

    std::span<uint8> slice(std::size_t start, std::size_t len) { return {ptr+start, len}; }
    std::span<uint8> contents()                                { return {ptr, len}; }
};

} // namespace Util
