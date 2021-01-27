#include "easyrandom.hpp"

#include "unsigned.hpp"

namespace Util {

GenType generator;
std::uniform_int_distribution<uint8> dist8;
std::mutex rnd_mutex;

void seed()
{
    std::random_device rd;
    generator.seed(rd());
}

uint8 random8()
{
    std::lock_guard<std::mutex> lock(rnd_mutex);
    return dist8(generator);
}

} // namespace Util

