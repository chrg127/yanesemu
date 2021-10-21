#pragma once

/* Basically a wrapper API around <random>. It's lazy initialized and thread-safe. */

#include <cstdint>

namespace util {

void seed();
uint8_t random8();

} // namespace util