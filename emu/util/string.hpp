#pragma once

/* Common string routines. */

#include <cstring>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace str {

/* Splits a string into a vector of strings. */
std::vector<std::string> split(const std::string &s, int delim = ',');
std::vector<std::string> split_lines(const std::string &s, int col);
std::string trim(const std::string &s);
void trim_inplace(std::string &s);

/* Converts a string to a number. */
template <typename T = int>
std::optional<T> _conv(const char *start, const char *end, unsigned base = 10)
{
    static_assert(std::is_integral_v<T>, "T must be an integral numeric type");
    T value = 0;
    auto res = std::from_chars(start, end, value, base);
    if (res.ec != std::errc() || res.ptr != end)
        return std::nullopt;
    return value;
}

template <typename T = int, typename TStr = std::string>
std::optional<T> conv(const TStr &str, unsigned base = 10)
{
    if constexpr(std::is_same<std::decay_t<TStr>, char *>::value)
        return _conv<T>(str, str + std::strlen(str), base);
    else
        return _conv<T>(str.data(), str.data() + str.size(), base);
}

// Check for space, no locale support.
inline bool is_space(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

} // namespace Util
