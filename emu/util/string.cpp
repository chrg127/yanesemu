#include "string.hpp"

#include <algorithm>
#include <utility>

namespace str {

std::vector<std::string> split(const std::string &s, int delim)
{
    std::size_t i = 0, p = 0;
    std::vector<std::string> res;

    while (i != s.size()) {
        p = s.find(delim, i);
        if (p == s.npos) {
            // no delimiter found: end
            res.emplace_back(s.begin() + i, s.end());
            break;
        }
        // get substring
        res.emplace_back(s.begin() + i, s.begin() + p);
        i = p+1;
    }
    return res;
}

std::vector<std::string> split_lines(const std::string &s, int col)
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

std::string trim(const std::string &s)
{
    auto i = std::find_if_not(s.begin(),  s.end(),  is_space);
    auto j = std::find_if_not(s.rbegin(), s.rend(), is_space).base();
    return std::string(i, j);
}

void trim_inplace(std::string &s)
{
    auto j = std::find_if_not(s.rbegin(), s.rend(), is_space).base();
    s.erase(j, s.end());
    auto i = std::find_if_not(s.begin(), s.end(), is_space);
    s.erase(s.begin(), i);
}

} // namespace Util
