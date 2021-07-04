#ifndef UTILS_STRINGOPS_HPP_INCLUDED
#define UTILS_STRINGOPS_HPP_INCLUDED

/* Common string routines. */

#include <cstring>
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
std::optional<T> _strconv(const char *start, const char *end, unsigned base = 10)
{
    static_assert(std::is_integral_v<T>, "T must be an integral numeric type");
    T value = 0;
    auto res = std::from_chars(start, end, value, base);
    if (res.ec != std::errc() || res.ptr != end)
        return std::nullopt;
    return value;
}

template <typename T = int, typename TStr = std::string>
std::optional<T> strconv(const TStr &str, unsigned base = 10)
{
    if constexpr(std::is_same<std::decay_t<TStr>, char *>::value)
        return _strconv<T>(str, str + std::strlen(str), base);
    else
        return _strconv<T>(str.data(), str.data() + str.size(), base);
}

// Check for space, no locale support.
inline bool is_space(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

} // namespace Util

#endif
