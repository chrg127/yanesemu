#include <emu/util/stringops.hpp>

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

} // namespace Util
