#pragma once

#include <cstdio>
#include <string>
#include <optional>

namespace io {

enum class Access { Read, Write, Modify, Append, };

class File {
    FILE *fp = nullptr;
    std::string name;

    File(FILE *f, std::string &&s) : fp{f}, name{std::move(s)} {}

public:
    ~File()
    {
        if (!fp || fp == stdin || fp == stdout || fp == stderr)
            return;
        std::fclose(fp);
        fp = nullptr;
        name.erase();
    }

    File(File &&f) noexcept { operator=(std::move(f)); }
    File & operator=(File &&f) noexcept
    {
        std::swap(fp, f.fp);
        std::swap(name, f.name);
        return *this;
    }

    static std::optional<File> open(std::string_view pathname, Access access)
    {
        FILE *fp = nullptr;
        switch (access) {
        case Access::Read:   fp = fopen(pathname.data(), "rb"); break;
        case Access::Write:  fp = fopen(pathname.data(), "rb"); break;
        case Access::Modify: fp = fopen(pathname.data(), "rb"); break;
        case Access::Append: fp = fopen(pathname.data(), "rb"); break;
        }
        if (!fp)
            return std::nullopt;
        return File{ fp, std::string(pathname) };
    }

    static File assoc(FILE *fp) { return { fp, "" }; }

    bool get_word(std::string &str)
    {
        auto is_delim = [](int c) { return c == '\n' || c == ' '  || c == '\t'; };
        auto is_space = [](int c) { return c == ' '  || c == '\t' || c == '\r'; };
        str.erase();
        int c;
        while (c = getc(), is_space(c) && c != EOF)
            ;
        ungetc(c);
        while (c = getc(), !is_delim(c) && c != EOF)
            str += c;
        ungetc(c);
        return !(c == EOF);
    }

    bool get_line(std::string &str, int delim = '\n')
    {
        auto is_space = [](int c) { return c == ' '  || c == '\t' || c == '\r'; };
        str.erase();
        int c;
        while (c = getc(), is_space(c) && c != EOF)
            ;
        ungetc(c);
        while (c = getc(), c != delim && c != EOF)
            str += c;
        return !(c == EOF);
    }

    std::string filename() const noexcept { return name; }
    FILE *data() const noexcept           { return fp; }
    int getc()                            { return std::fgetc(fp); }
    int ungetc(int c)                     { return std::ungetc(c, fp); }
};

} // namespace io
