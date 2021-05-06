#include "file.hpp"

#include "debug.hpp"

static FILE *open_file(const char *name, Util::Access access)
{
    switch (access) {
    case Util::Access::READ:   return fopen(name, "rb");
    case Util::Access::WRITE:  return fopen(name, "wb");
    case Util::Access::APPEND: return fopen(name, "ab");
    case Util::Access::MODIFY: return fopen(name, "rb+");
    }
    panic("invalid value passed to open_file");
}

static bool is_space(int c)
{
    return c == ' ' || c == '\t';
};

namespace Util {

bool File::open(std::string_view pathname, Access access)
{
    close();
    std::string pathname_copied = std::string(pathname);
    fp = open_file(pathname_copied.c_str(), access);
    if (!fp)
        return false;
    name = pathname_copied;
    return true;
}

void File::reopen(Access access)
{
    switch (access) {
    case Util::Access::READ:   fp = freopen(name.c_str(), "rb",  fp); break;
    case Util::Access::WRITE:  fp = freopen(name.c_str(), "wb",  fp); break;
    case Util::Access::APPEND: fp = freopen(name.c_str(), "ab",  fp); break;
    case Util::Access::MODIFY: fp = freopen(name.c_str(), "rb+", fp); break;
    }
}

/* getword() and getline() have different enough semantics that we really
 * should not try to generalize them */
bool File::getword(std::string &str)
{
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
    std::string contents;
    int c;
    contents.reserve(filesize());
    while (c = getc(), c != EOF)
        contents += c;
    return contents;
}

} // namespace Util

