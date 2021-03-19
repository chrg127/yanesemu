#ifndef UTILS_STRINGOPS_HPP_INCLUDED
#define UTILS_STRINGOPS_HPP_INCLUDED

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace Util {

std::vector<std::string> strsplit(const std::string &s, const int delim = ',');
std::optional<uint64_t> strtohex(const std::string &str, unsigned size = 0);

} // namespace Util

#endif
