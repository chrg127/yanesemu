#ifndef NESROM_H_INCLUDED
#define NESROM_H_INCLUDED

#include <cstdio>
#include <cstdint>

#define HEADER_LEN 16

class RomFile {
    uint8_t header[16];

public:
    FILE *file;

    RomFile()
    {
        file = NULL;
    }

    ~RomFile()
    {
        close();
    }

    int open(char *name);
    void close();
    bool eof()
    {
        return std::feof(file);
    }
};

#endif
