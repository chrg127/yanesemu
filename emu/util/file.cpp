#include <emu/util/file.hpp>

#include <cstring>
#include <cerrno>

namespace Util {

static std::string get_errstr()
{
#ifdef _WIN32
    char buf[256];
    strerror_s(buf, sizeof(buf), errno);
#elif defined(__linux__)
#  ifdef _GNU_SOURCE
    char tmpbuf[1];
    char *buf = strerror_r(errno, tmpbuf, sizeof(tmpbuf));
#  else
    char buf[256];
    strerror_r(errno, buf, sizeof(buf));
#  endif
#endif
    return std::string(buf);
}

bool File::open(const std::string_view pathname, const Mode filemode)
{
    close();
    switch (filemode) {
    case Mode::READ:   filbuf = std::fopen(pathname.data(), "rb");  break;
    case Mode::WRITE:  filbuf = std::fopen(pathname.data(), "wb");  break;
    case Mode::MODIFY: filbuf = std::fopen(pathname.data(), "rb+"); break;
    case Mode::APPEND: filbuf = std::fopen(pathname.data(), "ab");  break;
    }
    if (!filbuf) {
        errstr = get_errstr();
        return false;
    }
    filname = std::string(pathname);
    // std::fseek(filbuf, 0, SEEK_END);
    // filesize = std::ftell(filbuf);
    // std::fseek(filbuf, 0, SEEK_SET);
    return true;
}

/* getword and getline have different enough semantics that we really
 * should not try to generalize them */
bool File::getword(std::string &str)
{
    const auto is_space = [](int c) { return c == ' ' || c == '\t'; };
    const auto isdelim = [](int c) { return c == '\n' || c == ' ' || c == '\t'; };
    int c;

    str.erase();
    while (c = getc(), is_space(c) && c != EOF)
        ;
    ungetc(c);
    while (c = getc(), !isdelim(c) && c != EOF)
        str += c;
    ungetc(c);
    return !(c == EOF);
}

bool File::getline(std::string &str, int delim)
{
    const auto is_space = [](int c) { return c == ' ' || c == '\t'; };
    int c;

    str.erase();
    while (c = getc(), is_space(c) && c != EOF)
        ;
    ungetc(c);
    while (c = getc(), c != delim && c != EOF)
        str += c;
    return !(c == EOF);
}

std::string File::getall()
{
    std::string str;
    for (std::string tmp; getline(tmp); )
        str += tmp + '\n';
    return str;
}

} // namespace Util

