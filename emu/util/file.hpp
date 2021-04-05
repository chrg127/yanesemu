#ifndef UTIL_FILE_HPP_INCLUDED
#define UTIL_FILE_HPP_INCLUDED

/* A File class that C-Style file handling, but uses RAII
 * and std::string.
 * The File object itself can be empty, that is, it may
 * not have a valid file handle inside. This is intended
 * and can be tested using operator bool(). */

#include <cstdio>
#include <string>
#include <string_view>

namespace Util {

enum class Access {
    READ, WRITE, MODIFY, APPEND,
};

std::string syserr();

class File {
    FILE *fp = nullptr;
    std::string filname;

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
        std::swap(fp, f.fp);
        std::swap(filname, f.filname);
        return *this;
    }

    explicit operator bool() { return fp; }

    bool open(std::string_view pathname, Access access);
    long filesize() const;
    bool getword(std::string &str);
    bool getline(std::string &str, int delim = '\n');
    std::string getall();

    int close()
    {
        if (!fp || fp == stdin || fp == stdout || fp == stderr)
            return 0;
        int ret = fclose(fp);
        fp = nullptr;
        filname.erase();
        return ret;
    }

    FILE *release()
    {
        filname.erase();
        FILE *ret = fp;
        fp = nullptr;
        return ret;
    }

    void assoc(FILE *buf) { fp = buf; }
    bool eof()                        { return !fp || (std::feof(fp) != 0); }
    bool error()                      { return !fp || (std::ferror(fp) != 0); }
    int flush()                       { return std::fflush(fp); }
    int seek(long offset, int origin) { return std::fseek(fp, offset, origin); }
    int fd()                          { return fileno(fp); }
    std::string filename() const      { return filname; }
    FILE *data() const                { return fp; }

    std::size_t bread(void *buf, std::size_t nb)  { return std::fread(buf, 1, nb, fp); }
    int getc()                                    { return std::fgetc(fp); }
    int ungetc(int c)                             { return std::ungetc(c, fp); }
    std::size_t bwrite(void *buf, std::size_t nb) { return std::fwrite(buf, 1, nb, fp); }
    int putc(char c)                              { return std::fputc(c, fp); }
};

} // namespace Util

#endif
