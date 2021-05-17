#include "stringops.hpp"

namespace Util {

std::vector<std::string> strsplit(const std::string &s, int delim)
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

std::string trim(const std::string &str)
{
    auto i = str.begin();
    auto j = str.end() - 1;
    while (is_space(*i))
        i++;
    while (is_space(*j))
        j--;
    return std::string(i, j+1);
}

std::string ltrim(const std::string &str)
{
    auto i = str.begin();
    while (is_space(*i))
        i++;
    return std::string(i, str.end());
}

std::string rtrim(const std::string &str)
{
    auto i = str.end() - 1;
    while (is_space(*i))
        i--;
    return std::string(str.begin(), i+1);
}

} // namespace Util
