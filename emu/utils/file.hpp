/* Simple, safe and inheritable self-closing File class. No Windows support (yet). */

#ifndef UTILS_FILE_HPP_INCLUDED
#define UTILS_FILE_HPP_INCLUDED

#include <cstdio>
#include <cstdarg>
#include <string>
#include <string_view>

namespace Utils {

enum class Mode {
    READ,
    WRITE,
    MODIFY,
    APPEND,
};

enum class BufMode {
    UNBUF,
    LINEBUF,
    FULLBUF,
};

class File {
protected:
    FILE *fbuf = nullptr;
    long filesize = 0;
    Mode mode = Mode::READ;
    std::string filename;

public:
    File() = default;
    File(std::string_view s, Mode m)
    { open(s, m); }
    File(FILE *f, Mode m)
    { assoc(f, m); }

    ~File()
    { close(); }

    // File(const File &f) = delete;
    File(File &&f)
    { operator=(std::move(f)); }

    // File &operator=(const File &f) = delete;
    File &operator=(File &&f);

    /* open/close functions */
    bool open(std::string_view s, Mode m);
    void close();
    bool assoc(FILE *f, Mode m);

    inline bool isopen() const
    { return fbuf ? true : false; }

    inline long size() const
    { return filesize; }

    inline bool eof()
    {
        if (!fbuf)
            return true;
        return std::feof(fbuf) == 0 ? false : true;
    }

    inline bool error()
    {
        if (!fbuf)
            return true;
        return ferror(fbuf) == 0 ? false : true;
    }

    inline int flush()
    {
        return (!fbuf || mode == Mode::READ) ?
            EOF : fflush(fbuf);
    }

    inline int seek(long offset, int origin)
    {
        return (!fbuf) ? 1 : fseek(fbuf, offset, origin);
    }

    inline void set_buffer_mode(BufMode m)
    {
        switch (m) {
        case BufMode::UNBUF: setvbuf(fbuf, nullptr, _IONBF, 0);
        case BufMode::LINEBUF: setvbuf(fbuf, nullptr, _IOLBF, 0);
        case BufMode::FULLBUF: setvbuf(fbuf, nullptr, _IOFBF, 0);
        }
    }

    inline std::string getfilename() const
    { return filename; }

    /*
     * strictly for compatibility with existing FILE APIs.
     * if state invalidation might be a concern, use releasefbuf()
     */
    inline FILE *getfbuf() const
    { return fbuf; }

    inline int getfd()
    { return fileno(fbuf); }

    inline FILE *releasefbuf()
    {
        FILE *toret = fbuf;
        mode = Mode::READ;
        filesize = 0;
        filename = "";
        fbuf = nullptr;
        return toret;
    }



    /* Read functions */
    inline std::size_t readb(void *where, std::size_t bn)
    {
        return (!fbuf || (mode != Mode::READ && mode != Mode::MODIFY)) ?
            EOF : std::fread(where, 1, bn, fbuf);
    }

    inline int getc()
    {
        return (!fbuf || (mode != Mode::READ && mode != Mode::MODIFY)) ?
            EOF : std::fgetc(fbuf);
    }

    inline int ungetc(int c)
    {
        return (!fbuf || (mode != Mode::READ && mode != Mode::MODIFY)) ?
            EOF : std::ungetc(c, fbuf);
    }

    bool getline(std::string &s, int delim = '\n');

    inline std::string getall()
    {
        std::string s, tmp;
        while (getline(tmp))
            s += tmp + '\n';
        return s;
    }



    /* write functions */
    inline std::size_t writeb(void *what, std::size_t nb)
    {
        return (!fbuf || mode == Mode::READ) ? 0 :
            std::fwrite(what, 1, nb, fbuf);
    }

    inline int putc(char c)
    {
        return (!fbuf || mode == Mode::READ) ?
            EOF : std::fputc(c, fbuf);
    }

    inline int putstr(std::string_view s)
    {
        return (!fbuf || mode == Mode::READ) ?
            EOF : std::fputs(s.data(), fbuf);
    }

#if defined(__GNUC__) || defined(__MINGW32__) || defined (__MINGW64__)
    __attribute__((format(printf, 2, 3)))
#endif
    inline int printf(const char *fmt, ...)
    {
        if (!fbuf || mode == Mode::READ)
            return 0;
        va_list ap;
        va_start(ap, fmt);
        int toret = vfprintf(fbuf, fmt, ap);
        va_end(ap);
        return toret;
    }
};

} // namespace IO

#endif
