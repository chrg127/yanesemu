#ifndef STRINGOPS_HPP_INCLUDED
#define STRINGOPS_HPP_INCLUDED

#include <string>
#include <vector>

namespace Utils {

// uncomment this to use the template version instead.
// advantage is being more type-safe. disadvantage is generating code bloat.
// #define STRINGOPS_STRPRINTF_USE_TEMPLATE_VER

#ifndef STRINGOPS_STRPRINTF_USE_TEMPLATE_VER
#if defined(__GNUC__) || defined(__MINGW32__) || defined (__MINGW64__)
    __attribute__((format(printf, 1, 2)))
#endif
std::string strprintf(const char *fmt, ...);

#else

// template <typename ... Args>
// std::string strprintf(const char *fmt, Args... args)
// {
//     size_t size;
//     char *buf;
//     size = snprintf(nullptr, 0, fmt, args...) + 1;
//     if (size <= 0)
//         return "";
//     buf = new char[size];
//     snprintf(buf, size, fmt, args...);
//     std::string s(buf);
//     delete[] buf;
//     return s;
// }

#endif

std::vector<std::string> strsplit(const std::string &s, int delim = ',');

}

#endif
