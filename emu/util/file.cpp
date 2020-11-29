#include <emu/util/file.hpp>

#include <utility>

namespace Util {

File &File::operator=(File &&f)
{
    fbuf = f.fbuf;
    f.fbuf = nullptr;
    std::swap(filesize, f.filesize);
    mode = f.mode;
    std::swap(filename, f.filename);
    return *this;
}

bool File::open(std::string_view s, Mode m)
{
    close();

    switch (m) {
    case Mode::READ:   fbuf = std::fopen(s.data(), "r");  break;
    case Mode::WRITE:  fbuf = std::fopen(s.data(), "w");  break;
    case Mode::MODIFY: fbuf = std::fopen(s.data(), "r+"); break;
    case Mode::APPEND: fbuf = std::fopen(s.data(), "a");  break;
    }
    if (!fbuf)
        return false;
    mode = m;
    filename = s;
    std::fseek(fbuf, 0, SEEK_END);
    filesize = std::ftell(fbuf);
    std::fseek(fbuf, 0, SEEK_SET);
    return true;
}

void File::close()
{
    if (!fbuf || fbuf == stdin || fbuf == stdout || fbuf == stderr)
        return;
    fclose(fbuf);
    fbuf = nullptr;
    filesize = 0;
}

bool File::assoc(FILE *f, Mode m)
{
    // sanitize checks
    if (f == stdin && m != Mode::READ)
        return false;
    if ((f == stdout || f == stderr) && m != Mode::WRITE)
        return false;
    fbuf = f;
    mode = m;
    if (f == stdin || f == stdout || f == stderr)
        filesize = 0;
    else {
        std::fseek(fbuf, 0, SEEK_END);
        filesize = std::ftell(fbuf);
        std::fseek(fbuf, 0, SEEK_SET);
    }
    return true;
}

bool File::getline(std::string &s, int delim)
{
    // could probably be optimized.
    int c;

    if (!fbuf || (mode != Mode::READ && mode != Mode::MODIFY))
        return false;
    s.erase();
    while (c = getc(), c != delim && c != EOF)
        s += c;
    return (c == EOF) ? false : true;
}

} // namespace Util

