#include "nesrom.h"
#include <cstdio>

int RomFile::open(char *name)
{
    file = std::fopen(name, "rb");
    return file == NULL;
}

void RomFile::close()
{
    if (file)
        fclose(file);
}

