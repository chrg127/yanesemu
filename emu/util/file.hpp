/* Simple, safe self-closing File class. No Windows support (yet). */

#ifndef UTILS_FILE_HPP_INCLUDED
#define UTILS_FILE_HPP_INCLUDED

#include <cstdio>
#include <cstdarg>
#include <string>
#include <string_view>
#include <fmt/core.h>

namespace Util {

class File {
    FILE *fbuf = nullptr;
    long filesize = 0;
public:
    enum class Mode {
        READ,
        WRITE,
        MODIFY,
        APPEND,
    };
private:
    Mode mode = Mode::READ;
    std::string file_name;
public:
    enum class BufMode {
        UNBUF,
        LINEBUF,
        FULLBUF,
    };

    File() = default;
    File(std::string_view s, Mode m) { open(s, m); }
    File(FILE *f, Mode m)            { assoc(f, m); }

    ~File() { close(); }

    File(const File &f) = delete;
    File(File &&f) { operator=(std::move(f)); }

    File & operator=(const File &f) = delete;
    File & operator=(File &&f);

    /* open/close functions */
    bool open(std::string_view s, Mode m);
    void close();
    bool assoc(FILE *f, Mode m);

    bool isopen() const  { return fbuf ? true : false; }
    explicit operator bool() { return isopen(); }

    long size() const    { return filesize; }
    bool eof()           { return !fbuf ? true : std::feof(fbuf) != 0 ? true : false; }
    bool error()         { return !fbuf ? true : ferror(fbuf)    != 0 ? true : false; }
    int flush()          { return fflush(fbuf); }
    int seek(long offset, int origin) { return fseek(fbuf, offset, origin); }
    std::string filename() const { return file_name; }
    // strictly for compatibility with existing FILE APIs.
    // if state invalidation might be a concern, use releasefbuf()
    FILE *getfbuf() const { return fbuf; }
    int getfd()           { return fileno(fbuf); }

    void set_buffer_mode(BufMode m)
    {
        switch (m) {
        case BufMode::UNBUF: setvbuf(fbuf, nullptr, _IONBF, 0);
        case BufMode::LINEBUF: setvbuf(fbuf, nullptr, _IOLBF, 0);
        case BufMode::FULLBUF: setvbuf(fbuf, nullptr, _IOFBF, 0);
        }
    }

    FILE *release()
    {
        FILE *toret = fbuf;
        mode = Mode::READ;
        filesize = 0;
        file_name = "";
        fbuf = nullptr;
        return toret;
    }

    // read functions
    std::size_t readb(void *where, std::size_t bn) { return std::fread(where, 1, bn, fbuf); }
    int getc()                                     { return std::fgetc(fbuf); }
    int ungetc(int c)                              { return std::ungetc(c, fbuf); }
    bool getline(std::string &s, int delim = '\n');

    std::string getall()
    {
        std::string s;
        for (std::string tmp; getline(tmp); )
            s += tmp + '\n';
        return s;
    }

    // write functions
    std::size_t writeb(void *what, std::size_t nb) { return std::fwrite(what, 1, nb, fbuf); }
    int putc(char c)                               { return std::fputc(c, fbuf); }
    void putstr(std::string_view &&s)                 { return fmt::print(fbuf, "{}", s); }
    void putstr(std::string &&s)                      { return fmt::print(fbuf, "{}", s); }

    template <typename... T>
    void print(std::string &&fmt, T... args)
    {
        fmt::print(fbuf, fmt, args...);
    }
};

} // namespace Util

#endif
