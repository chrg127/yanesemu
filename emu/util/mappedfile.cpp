#include "mappedfile.hpp"

#ifdef __linux__
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#error "Platforms without mmap() are still unsupported."
#endif

namespace io {

#ifdef __linux__

std::optional<MappedFile> MappedFile::open(std::string_view pathname)
{
    int fd = ::open(pathname.data(), O_RDWR);
    if (fd < 0)
        return std::nullopt;
    struct stat statbuf;
    int err = fstat(fd, &statbuf);
    if (err < 0)
        return std::nullopt;
    auto *ptr = (u8 *) mmap(nullptr, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
        return std::nullopt;
    close(fd);
    return MappedFile{ptr, static_cast<std::size_t>(statbuf.st_size), pathname};
}

MappedFile::~MappedFile()
{
    if (ptr) ::munmap(ptr, len);
}

#endif

} // namespace io
