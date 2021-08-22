#pragma once

/* Basically a wrapper API around <random>.
 * It's thread-safe, or at least it's supposed to be.
 * Call seed() in main().  */

#include <cstdint>

namespace util {

void seed();
uint8_t random8();

} // namespace Util
