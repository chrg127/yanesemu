#ifndef UTILS_STRINGOPS_HPP_INCLUDED
#define UTILS_STRINGOPS_HPP_INCLUDED

#include <string>
#include <vector>
#include <optional>
#include <charconv>

namespace Util {

std::vector<std::string> strsplit(const std::string &s, const int delim = ',');

template <typename T>
std::optional<T> strconv(const std::string &str, unsigned expected_size = 0, unsigned base = 0)
{
    if (str.size() == 0 || (expected_size != 0 && str.size() != expected_size))
        return std::nullopt;
    T value;
    auto res = std::from_chars(str.data(), str.data() + str.size(), value, base);
    if (res.ec != std::errc())
        return std::nullopt;
    return value;
}

} // namespace Util

#endif
