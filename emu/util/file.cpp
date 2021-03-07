#include <emu/util/file.hpp>

#include <cstring>
#include <cerrno>

namespace Util {

static std::string get_errstr()
{
#ifdef _GNU_SOURCE
    char tmpbuf[1];
    char *buf = strerror_r(errno, tmpbuf, sizeof(tmpbuf));
#else
    char buf[256];
    strerror_r(errno, buf, sizeof(buf));
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

} // namespace Util

