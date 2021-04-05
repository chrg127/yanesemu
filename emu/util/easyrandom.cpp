#include "easyrandom.hpp"

#include <mutex>
#include <random>

namespace Util {

using GenType = std::mt19937;
GenType generator;
std::uniform_int_distribution<uint8_t> dist8;
std::mutex rnd_mutex;

void seed()
{
    std::random_device rd;
    generator.seed(rd());
}

uint8_t random8()
{
    std::lock_guard<std::mutex> lock(rnd_mutex);
    return dist8(generator);
}

} // namespace Util
