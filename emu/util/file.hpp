#ifndef UTIL_FILE_HPP_INCLUDED
#define UTIL_FILE_HPP_INCLUDED

#include <cstdio>
#include <string>
#include <string_view>
// #include <fmt/core.h>

namespace Util {

enum class Access {
    READ, WRITE, MODIFY, APPEND,
};

std::string syserr();

class File {
    FILE *filbuf = nullptr;
    std::string filname = "";

public:
    File() = default;
    File(std::string_view pathname, Access access) { open(pathname, access); }
    File(FILE *buf) { assoc(buf); }
    ~File() { close(); }

    File(const File &f) = delete;
    File(File &&f) { operator=(std::move(f)); }
    File & operator=(const File &f) = delete;
    File & operator=(File &&f)
    {
        std::swap(filbuf, f.filbuf);
        std::swap(filname, f.filname);
        return *this;
    }

    explicit operator bool() { return filbuf; }

    bool open(std::string_view pathname, Access access);
    long filesize() const;
    bool getword(std::string &str);
    bool getline(std::string &str, int delim = '\n');
    std::string getall();

    int close()
    {
        if (!filbuf || filbuf == stdin || filbuf == stdout || filbuf == stderr)
            return 0;
        int ret = fclose(filbuf);
        filbuf = nullptr;
        filname.erase();
        return ret;
    }

    FILE *release()
    {
        filname.erase();
        FILE *ret = filbuf;
        filbuf = nullptr;
        return ret;
    }

    void assoc(FILE *buf) { filbuf = buf; }
    bool eof()                        { return !filbuf || (std::feof(filbuf)   != 0); }
    bool error()                      { return !filbuf || (std::ferror(filbuf) != 0); }
    int flush()                       { return std::fflush(filbuf); }
    int seek(long offset, int origin) { return std::fseek(filbuf, offset, origin); }
    int fd()                          { return fileno(filbuf); }
    std::string filename() const      { return filname; }
    FILE *data() const                { return filbuf; }

    std::size_t bread(void *buf, std::size_t nb)  { return std::fread(buf, 1, nb, filbuf); }
    int getc()                                    { return std::fgetc(filbuf); }
    int ungetc(int c)                             { return std::ungetc(c, filbuf); }
    std::size_t bwrite(void *buf, std::size_t nb) { return std::fwrite(buf, 1, nb, filbuf); }
    int putc(char c)                              { return std::fputc(c, filbuf); }

    /*
    template <typename... T> void print(std::string &&fmt, T... args)
    {
        fmt::print(filbuf, fmt, args...);
    }
    */
};

} // namespace Util

#endif
