#ifndef UTIL_FILE_HPP_INCLUDED
#define UTIL_FILE_HPP_INCLUDED

/* A File class. Internally uses the C FILE * type. */

#include <cstdio>
#include <string>
#include <optional>
#include "heaparray.hpp"
#include "unsigned.hpp"

namespace Util {

enum class Access {
    READ, WRITE, MODIFY, APPEND,
};

class File {
    FILE *fp = nullptr;
    std::string name;

    File() = default;

public:
    ~File();

    File(const File &f) = delete;
    File(File &&f) { operator=(std::move(f)); }
    File & operator=(const File &f) = delete;
    File & operator=(File &&f)
    {
        std::swap(fp, f.fp);
        std::swap(name, f.name);
        return *this;
    }

    // Create a File using ONLY these two functions
    static std::optional<File> open(std::string_view pathname, Access access);
    static File assoc(FILE *fp);

    bool getword(std::string &str);
    bool getline(std::string &str, int delim = '\n');
    std::string getall();

    std::string filename() const      { return name; }
    FILE *data() const                { return fp; }
    bool eof()                        { return !fp || (std::feof(fp) != 0); }
    bool error()                      { return !fp || (std::ferror(fp) != 0); }
    int flush()                       { return std::fflush(fp); }
    int seek(long offset, int origin) { return std::fseek(fp, offset, origin); }
    int fd()                          { return fileno(fp); }
    int getc()                                    { return std::fgetc(fp); }
    int ungetc(int c)                             { return std::ungetc(c, fp); }
    int putc(int c)                               { return std::fputc(c, fp); }
    std::size_t bread(void *buf, std::size_t nb)  { return std::fread(buf, 1, nb, fp); }
    std::size_t bwrite(void *buf, std::size_t nb) { return std::fwrite(buf, 1, nb, fp); }

    void reopen(Access access)
    {
        switch (access) {
        case Util::Access::READ:   fp = freopen(name.c_str(), "rb",  fp); break;
        case Util::Access::WRITE:  fp = freopen(name.c_str(), "wb",  fp); break;
        case Util::Access::APPEND: fp = freopen(name.c_str(), "ab",  fp); break;
        case Util::Access::MODIFY: fp = freopen(name.c_str(), "rb+", fp); break;
        }
    }

    FILE *release()
    {
        name.erase();
        FILE *ret = fp;
        fp = nullptr;
        return ret;
    }

    long filesize() const
    {
        long curr = std::ftell(fp);
        std::fseek(fp, 0L, SEEK_END);
        long size = std::ftell(fp);
        std::fseek(fp, curr, SEEK_SET);
        return size;
    }

    HeapArray<uint8> read_bytes(std::size_t nb)
    {
        HeapArray<uint8> arr(nb);
        auto nr = bread(arr.data(), nb);
        arr.shrink(nr);
        return arr;
    }

    template <typename TArr>
    void write_bytes(const TArr &buf)
    {
        bwrite(buf.data(), buf.size());
    }
};

std::optional<File> open_file(Access access);

} // namespace Util

#endif
