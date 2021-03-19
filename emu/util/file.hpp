#ifndef UTIL_FILE_HPP_INCLUDED
#define UTIL_FILE_HPP_INCLUDED

#include <cstdio>
#include <string>
#include <string_view>
#include <fmt/core.h>

namespace Util {

class File {
    FILE *filbuf = nullptr;
    std::string filname = "";
    std::string errstr = "";
    // long filesize = 0;

public:
    enum class Mode {
        READ, WRITE, MODIFY, APPEND,
    };

    File() = default;
    File(const std::string_view pathname, const Mode filemode) { open(pathname, filemode); }
    File(FILE *buf) { assoc(buf); }

    ~File() { close(); }

    File(const File &f) = delete;
    File & operator=(const File &f) = delete;

    File(File &&f) { operator=(std::move(f)); }
    File & operator=(File &&f)
    {
        std::swap(filbuf, f.filbuf);
        std::swap(filname, f.filname);
        // std::swap(filesize, f.filesize);
        return *this;
    }

    explicit operator bool() { return filbuf; }

    bool open(const std::string_view pathname, const Mode filemode);

    int close()
    {
        if (!filbuf || filbuf == stdin || filbuf == stdout || filbuf == stderr)
            return 0;
        int ret = fclose(filbuf);
        filbuf = nullptr;
        // filesize = 0;
        filname.erase();
        return ret;
    }

    void assoc(FILE *buf) { filbuf = buf; }
        // if (buf == stdin || buf == stdout || buf == stderr)
        //     filesize = 0;
        // else {
        //     std::fseek(filbuf, 0, SEEK_END);
        //     filesize = std::ftell(filbuf);
        //     std::fseek(filbuf, 0, SEEK_SET);
        // }

    bool eof()                        { return !filbuf || (std::feof(filbuf)   != 0); }
    bool error()                      { return !filbuf || (std::ferror(filbuf) != 0); }
    std::string error_str()           { return errstr; }
    int flush()                       { return std::fflush(filbuf); }
    int seek(long offset, int origin) { return std::fseek(filbuf, offset, origin); }
    int fd()                          { return fileno(filbuf); }
    // long size() const                 { return filesize; }
    std::string filename() const      { return filname; }
    FILE *internal_buffer() const     { return filbuf; }

    FILE *release()
    {
        filname.erase();
        // filesize = 0;
        FILE *ret = filbuf;
        filbuf = nullptr;
        return ret;
    }

    // read functions
    std::size_t bread(void *buf, std::size_t nb) { return std::fread(buf, 1, nb, filbuf); }
    int getc()                                   { return std::fgetc(filbuf); }
    int ungetc(int c)                            { return std::ungetc(c, filbuf); }

    bool getword(std::string &str);
    bool getline(std::string &str, int delim = '\n');
    std::string getall();

    // write functions
    std::size_t bwrite(void *buf, std::size_t nb) { return std::fwrite(buf, 1, nb, filbuf); }
    int putc(char c)                              { return std::fputc(c, filbuf); }

    template <typename T> void print(const T &&obj)
    {
        return fmt::print(filbuf, "{}", obj);
    }

    template <typename... T> void print(std::string &&fmt, T... args)
    {
        fmt::print(filbuf, fmt, args...);
    }
};

} // namespace Util

#endif
