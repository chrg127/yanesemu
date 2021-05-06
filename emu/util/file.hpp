#ifndef UTIL_FILE_HPP_INCLUDED
#define UTIL_FILE_HPP_INCLUDED

/* A File class. Internally uses the C FILE * type. The File object itself can be empty, that is, it may
 * not have a valid file handle inside. This is an intended state and can be tested using operator bool().
 *
 * Method documentation:
 * - File(): construct empty File.
 * - File(pathname, access), open(pathname, access):
 *   open File using pathname. The File may find an error while opening, to test it and print an error:
 *      File file{"test.txt", Access::READ};
 *      if (!file)
 *          perror("");
 * - File(f), assoc(f): associate FILE * f with this File object. The underlying
 *   FILE *, if present, is closed and the name is reset.
 * - ~File(), close(): closes the underlying FILE * object.
 * - Copy constructors are deleted, only move destructors should be used.
 * - eof(): test for End Of File.
 * - error(): test for error.
 * - flush(): flushes the file, if buffered.
 * - seek(offset, origin): seek to specified position. See fseek().
 * - fd(): get the underlying file descriptor.
 * - filename(): gets the file's name. This is the same as the pathname passed
 *   when opening the file.
 * - data(): gets the underlying FILE * object.
 * - release(): release the underlying FILE * object and transform the File
 *   object as if it closed the FILE *.
 * - filesize(): return the file's size.
 * - getword(str): gets a word from the file and return whether it's EOF.
 * - getline(str): same as above, but gets a string.
 * - getall(): get all of the file's contents and return it as a string.
 * - getc(): get a single character from the file.
 * - ungetc(c): put c back into the file's stream. See ungetc().
 * - putc(c): write c to file.
 * - bread(buf, nb): read nb bytes from file stream and put them into buf.
 *   Returns how many bytes were effectively read.
 * - bwrite(buf, nb): write nb bytes from file stream and put them into buf.
 *   Return how many bytes were effectively written.
 * - read_bytes(nb): same as bread(), but return a HeapArray instead.
 * - write_bytes(buf): same as bwrite(), but use a container instead.
 */

#include <cstdio>
#include <string>
#include "heaparray.hpp"
#include "unsigned.hpp"

namespace Util {

enum class Access {
    READ, WRITE, MODIFY, APPEND,
};

class File {
    FILE *fp = nullptr;
    std::string name;

public:
    File() = default;
    File(std::string_view pathname, Access access) { open(std::move(pathname), access); }
    File(FILE *f)                               { assoc(f); }

    ~File() { close(); }

    File(const File &f) = delete;
    File(File &&f) { operator=(std::move(f)); }
    File & operator=(const File &f) = delete;
    File & operator=(File &&f)
    {
        std::swap(fp, f.fp);
        std::swap(name, f.name);
        return *this;
    }

    explicit operator bool() { return !!fp; }

    bool open(std::string_view pathname, Access access);
    void reopen(Access access);

    bool eof()                        { return !fp || (std::feof(fp) != 0); }
    bool error()                      { return !fp || (std::ferror(fp) != 0); }
    int flush()                       { return std::fflush(fp); }
    int seek(long offset, int origin) { return std::fseek(fp, offset, origin); }
    int fd()                          { return fileno(fp); }
    std::string filename() const      { return name; }
    FILE *data() const                { return fp; }

    void assoc(FILE *f)
    {
        close();
        name = "";
        fp = f;
    }

    int close()
    {
        if (!fp || fp == stdin || fp == stdout || fp == stderr)
            return 0;
        int ret = fclose(fp);
        fp = nullptr;
        name.erase();
        return ret;
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

    bool getword(std::string &str);
    bool getline(std::string &str, int delim = '\n');
    std::string getall();

    int getc()                                    { return std::fgetc(fp); }
    int ungetc(int c)                             { return std::ungetc(c, fp); }
    int putc(int c)                               { return std::fputc(c, fp); }
    std::size_t bread(void *buf, std::size_t nb)  { return std::fread(buf, 1, nb, fp); }
    std::size_t bwrite(void *buf, std::size_t nb) { return std::fwrite(buf, 1, nb, fp); }

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

} // namespace Util

#endif
