#ifndef UTILS_STRINGOPS_HPP_INCLUDED
#define UTILS_STRINGOPS_HPP_INCLUDED

#include <string>
#include <vector>

namespace Util {

// well this header sure feels useless now...
std::vector<std::string> strsplit(const std::string &s, const int delim = ',');

} // namespace Util

#endif
