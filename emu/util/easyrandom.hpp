#include <random>
#include <mutex>
#include "unsigned.hpp"

/* Basically a wrapper API around <random>
 * It's thread-safe, or at least it's supposed to be.
 * Call seed() in main().
 */
namespace Util {

using GenType = std::mt19937;
extern GenType generator;
extern std::mutex rnd_mutex;

void seed();
uint8 random8();

template <typename T> // T = number type
T random_between(T min, T max)
{
    std::lock_guard<std::mutex> lock(rnd_mutex);
    std::uniform_int_distribution<T> dist(min, max);
    return dist(generator);
}

} // namespace Util
