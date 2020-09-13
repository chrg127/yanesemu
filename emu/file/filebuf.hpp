/* Simple, safe and inheritable self-closing File class. No Windows support. */

#ifndef FILEBUF_HPP_INCLUDED
#define FILEBUF_HPP_INCLUDED

#include <cstdio>
#include <cstddef>
#include <cstdarg>
#include <string>

namespace File {

enum class Mode {
    READ,
    WRITE,
    MODIFY,
    APPEND,
};

class FileBuf {
protected:
    FILE *buf = nullptr;
    long filesize = 0;
    Mode mode = Mode::READ;
    std::string filename;

public:
    FileBuf() = default;
    FileBuf(const std::string &s, Mode m)
    { open(s, m); }
    FileBuf(FILE *f, Mode m)
    { assoc(f, m); }

    ~FileBuf()
    { close(); }

    /* open/close functions */
    bool open(const std::string &s, Mode m);
    void close();
    bool assoc(FILE *f, Mode m);
    // bool reopen();

    inline bool isopen()
    { return buf ? true : false; }

    inline long size() const
    { return filesize; }

    inline bool eof()
    {
        if (!buf)
            return true;
        return std::feof(buf) == 0 ? false : true;
    }

    inline bool error()
    {   
        if (!buf)
            return true;
        return ferror(buf) == 0 ? false : true;
    }

    inline int flush()
    {
        if (!buf || mode == Mode::READ)
            return EOF;
        return fflush(buf);
    }

    inline int seek(long offset, int origin)
    {
        if (!buf)
            return 1;
        return fseek(buf, offset, origin);
    }
    
    /* 
     * strictly for compatibility with existing FILE APIs.
     * if state invalidation might be a concern, use releasebuf()
     */
    inline FILE *getbuf()
    { return buf; }

    inline int getfd()
    { return fileno(buf); }

    inline FILE *releasebuf()
    {
        mode = Mode::READ;
        filesize = 0;
        filename = "";
        return buf;
    }



    /* Read functions */
    inline size_t readb(void *inbuf, size_t bn)
    {
        if (!buf || (mode != Mode::READ && mode != Mode::MODIFY))
            return EOF;
        return std::fread(inbuf, 1, bn, buf);
    }

    inline int getc()
    {
        if (!buf || (mode != Mode::READ && mode != Mode::MODIFY))
            return EOF;
        return std::fgetc(buf);
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
    inline size_t writeb(void *what, size_t nb)
    {
        if (!buf || mode == Mode::READ)
            return 0;
        return std::fwrite(what, 1, nb, buf);
    }

    inline int putc(char c)
    {
        if (!buf || mode == Mode::READ)
            return EOF;
        return std::fputc(c, buf);
    }
    
    inline int putstr(const std::string &s)
    {
        if (!buf || mode == Mode::READ)
            return EOF;
        return std::fputs(s.c_str(), buf);
    }

    inline int printf(const char *fmt, ...)
#if defined(__GNUC__) || defined(__MINGW32__) || defined (__MINGW64__)
    __attribute__((format(printf, 2, 3)))
#endif
    {
        if (!buf || mode == Mode::READ)
            return 0;
        va_list ap;
        va_start(ap, fmt);
        int toret = vfprintf(buf, fmt, ap);
        va_end(ap);
        return toret;
    }
};

} // namespace File

#endif
