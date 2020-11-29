#include <emu/util/stringops.hpp>

#include <cstdio>
#include <cstdarg>
#include <cstdlib>

namespace Util {

#ifndef STRINGOPS_STRPRINTF_USE_TEMPLATE_VER
#if defined(__GNUC__) || defined(__MINGW32__) || defined (__MINGW64__)
    __attribute__((format(printf, 1, 2)))
#endif
std::string strprintf(const char *fmt, ...)
{
    va_list tmp, ap;
    size_t size;
    char *buf;

    va_start(tmp, fmt);
    size = vsnprintf(nullptr, 0, fmt, tmp) + 1;
    va_end(tmp);
    if (size <= 0)
        return "";
    buf = new char[size];
    va_start(ap, fmt);
    vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    std::string s(buf);
    delete[] buf;
    return s;
}
#endif

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

} // namespace Util
