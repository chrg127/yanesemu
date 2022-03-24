#pragma once

#include <algorithm>
#include <cstring>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace str {

inline bool is_space(int c) { return c == ' ' || c == '\t' || c == '\r'; }

inline std::vector<std::string_view> split(std::string_view s, char delim = ',')
{
    std::vector<std::string_view> res;
    for (auto i = 0l, p = 0l; i != s.size(); i = p+1) {
        p = s.find(delim, i);
        if (p == s.npos) {
            res.emplace_back(s.substr(i, s.size() - i));
            break;
        }
        res.emplace_back(s.substr(i, p-i));
        i = p+1;
    }
    return res;
}

inline std::vector<std::string> split(const std::string &s, char delim = ',')
{
    std::vector<std::string> res;
    for (auto i = 0l, p = 0l; i != s.size(); i = p+1) {
        p = s.find(delim, i);
        if (p == s.npos) {
            res.emplace_back(s.begin() + i, s.end());
            break;
        }
        res.emplace_back(s.begin() + i, s.begin() + p);
        i = p+1;
    }
    return res;
}

inline std::vector<std::string> split_lines(const std::string &s, int col)
{
    std::vector<std::string> result;
    auto it = s.begin();
    while (it != s.end()) {
        it = std::find_if_not(it, s.end(), is_space);
        auto start = it;
        it += std::min(size_t(s.end() - it), size_t(col));
        it = std::find_if(it, s.end(), is_space);
        result.emplace_back(start, it);
    }
    return result;
}


inline std::string trim(const std::string &s)
{
    auto i = std::find_if_not(s.begin(),  s.end(),  is_space);
    auto j = std::find_if_not(s.rbegin(), s.rend(), is_space).base();
    return {i, j};
}

inline void trim_in_place(std::string &s)
{
    auto j = std::find_if_not(s.rbegin(), s.rend(), is_space).base();
    s.erase(j, s.end());
    auto i = std::find_if_not(s.begin(), s.end(), is_space);
    s.erase(s.begin(), i);
}


template <typename T = int, typename TStr = std::string>
inline std::optional<T> to_num(const TStr &str, unsigned base = 10)
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "T must be a number");
    auto helper = [](const char *start, const char *end, unsigned base) -> std::optional<T> {
        T value = 0;
        std::from_chars_result res;
        if constexpr(std::is_same_v<T, float>)
            res = std::from_chars(start, end, value);
        else
            res = std::from_chars(start, end, value, base);
        if (res.ec != std::errc() || res.ptr != end)
            return std::nullopt;
        return value;
    };
    if constexpr(std::is_same<std::decay_t<TStr>, char *>::value)
        return helper(str, str + std::strlen(str), base);
    else
        return helper(str.data(), str.data() + str.size(), base);
}

} // namespace str
