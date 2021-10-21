#pragma once

/* A File class. Internally uses the C FILE * type. */

#include <cstdio>
#include <string>
#include <optional>
#include <emu/util/array.hpp>
#include <emu/util/uint.hpp>

namespace io {

enum class Access {
    READ, WRITE, MODIFY, APPEND,
};

class File {
    FILE *fp = nullptr;
    std::string name;

    File() = default;

public:
    ~File();

    File(File &&f) noexcept { operator=(std::move(f)); }
    File & operator=(File &&f) noexcept
    {
        std::swap(fp, f.fp);
        std::swap(name, f.name);
        return *this;
    }

    // File's are created using these two functions.
    // The first opens a file using a string, the second associates an existing file (used for stdin, stdout, stderr).
    static std::optional<File> open(std::string_view pathname, Access access);
    static File assoc(FILE *fp);

    bool getword(std::string &str);
    bool getline(std::string &str, int delim = '\n');

    std::string filename() const                  { return name; }
    FILE *data() const                            { return fp; }
    int getc()                                    { return std::fgetc(fp); }
    int ungetc(int c)                             { return std::ungetc(c, fp); }
};

} // namespace io