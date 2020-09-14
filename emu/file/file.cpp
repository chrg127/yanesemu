#include <emu/file/filebuf.hpp>

namespace IO {

bool FileBuf::open(const std::string &s, Mode m)
{
    close();

    switch (m) {
    case Mode::READ:   buf = std::fopen(s.c_str(), "r");  break;
    case Mode::WRITE:  buf = std::fopen(s.c_str(), "w");  break;
    case Mode::MODIFY: buf = std::fopen(s.c_str(), "r+"); break;
    case Mode::APPEND: buf = std::fopen(s.c_str(), "a");  break;
    }
    if (!buf)
        return true;
    mode = m;
    filename = s;
    std::fseek(buf, 0, SEEK_END);
    filesize = std::ftell(buf);
    std::fseek(buf, 0, SEEK_SET);
    return false;
}

void FileBuf::close()
{
    if (!buf || buf == stdin || buf == stdout || buf == stderr)
        return;
    fclose(buf);
    buf = nullptr;
    filesize = 0;
}

bool FileBuf::assoc(FILE *f, Mode m)
{
    // sanitize checks
    if (f == stdin && m != Mode::READ)
        return false;
    if ((f == stdout || f == stderr) && m != Mode::WRITE)
        return false;
    buf = f;
    mode = m;
    if (f == stdin || f == stdout || f == stderr)
        filesize = 0;
    else {
        std::fseek(buf, 0, SEEK_END);
        filesize = std::ftell(buf);
        std::fseek(buf, 0, SEEK_SET);
    }
    return true;
}

bool FileBuf::getline(std::string &s, int delim)
{
    // could probably be optimized.
    int c;
    
    if (!buf || (mode != Mode::READ && mode != Mode::MODIFY))
        return false;
    s.erase();
    while (c = getc(), c != delim && c != EOF)
        s += c;
    if (c == EOF)
        return false;
    return true;
}

} // namespace File

