#include <emu/util/stringops.hpp>

#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <charconv>

namespace Util {

std::vector<std::string> strsplit(const std::string &s, const int delim)
{
    std::size_t i = 0, p = 0;
    std::vector<std::string> res;

    while (i != s.size()) {
        p = s.find(delim, i);
        // no delimiter found?
        if (p == s.npos) {
            res.emplace_back(s.begin() + i, s.end());
            break;
        }
        // get substring
        res.emplace_back(s.begin() + i, s.begin() + p);
        i = p+1;
    }
    return res;
}

std::optional<uint64_t> strtohex(const std::string &str, unsigned size)
{
    if (str.size() == 0 || str.size() > 16 || (size != 0 && str.size() != size))
       return std::nullopt;
    uint64_t value;
    auto res = std::from_chars(str.data(), str.data() + str.size(), value, 16);
    if (res.ec != std::errc())
        return std::nullopt;
    return value;
}

} // namespace Util
