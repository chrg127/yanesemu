#include <emu/util/file.hpp>

#include <cerrno>
#include <system_error>
#include <emu/util/debug.hpp>

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

std::string syserr()
{
    return std::make_error_code(static_cast<std::errc>(errno)).message();
}

bool File::open(std::string_view pathname, Access access)
{
    close();
    fp = open_file(pathname.data(), access);
    if (!fp)
        return false;
    filname = std::string(pathname);
    return true;
}

long File::filesize() const
{
    long curr = std::ftell(fp);
    std::fseek(fp, 0L, SEEK_END);
    long size = std::ftell(fp);
    std::fseek(fp, curr, SEEK_SET);
    return size;
}

/* getword and getline have different enough semantics that we really
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

