#include "file.hpp"

#include <cassert>

static bool is_space(int c)
{
    return c == ' ' || c == '\t';
}

namespace Util {

std::optional<File> File::open(std::string_view pathname, Access access)
{
    const auto open = [](const char *name, Access access) -> FILE *
    {
        switch (access) {
        case Util::Access::READ:   return fopen(name, "rb");
        case Util::Access::WRITE:  return fopen(name, "wb");
        case Util::Access::APPEND: return fopen(name, "ab");
        case Util::Access::MODIFY: return fopen(name, "rb+");
        default: return nullptr;
        }
    };
    File file;
    file.name = std::string(pathname);
    file.fp = open(file.name.c_str(), access);
    if (!file.fp)
        return std::nullopt;
    return file;
}

File File::assoc(FILE *fp)
{
    assert(fp);
    File file;
    file.fp = fp;
    file.name = "";
    return file;
}

File::~File()
{
    if (!fp || fp == stdin || fp == stdout || fp == stderr)
        return;
    fclose(fp);
    fp = nullptr;
    name.erase();
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

