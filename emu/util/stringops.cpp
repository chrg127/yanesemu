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

std::vector<std::string> strsplit(const std::string &s, int delim)
{
    std::vector<std::string> strvec;
    std::string tmp;

    for (auto &c : s) {
        if (c != delim)
            tmp += c;
        else {
            strvec.push_back(tmp);
            tmp.erase();
        }
    }
    return strvec;
}

} // namespace Util
