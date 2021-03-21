#ifndef UTILS_STRINGOPS_HPP_INCLUDED
#define UTILS_STRINGOPS_HPP_INCLUDED

#include <string>
#include <vector>
#include <optional>
#include <charconv>

namespace Util {

std::vector<std::string> strsplit(const std::string &s, const int delim = ',');

template <typename T>
std::optional<T> strconv(const std::string &str, unsigned base = 0)
{
    T value;
    auto res = std::from_chars(str.data(), str.data() + str.size(), value, base);
    if (res.ec != std::errc())
        return std::nullopt;
    return value;
}

} // namespace Util

#endif
