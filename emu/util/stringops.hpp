#ifndef UTILS_STRINGOPS_HPP_INCLUDED
#define UTILS_STRINGOPS_HPP_INCLUDED

/* Common string routines. */

#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace Util {

/* Splits a string into a vector of strings. */
std::vector<std::string> strsplit(const std::string &s, int delim = ',');

/* Converts a string to a number. */
template <typename T = int>
std::optional<T> strconv(const std::string &str, unsigned base = 0)
{
    T value;
    auto res = std::from_chars(str.data(), str.data() + str.size(), value, base);
    if (res.ec != std::errc())
        return std::nullopt;
    return value;
}

template <typename T = int>
std::optional<T> strconv(std::string_view str, unsigned base = 0)
{
    T value;
    auto res = std::from_chars(str.data(), str.data() + str.size(), value, base);
    if (res.ec != std::errc())
        return std::nullopt;
    return value;
}

} // namespace Util

#endif
