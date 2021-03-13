#ifndef UTIL_EASYRANDOM_HPP_INCLUDED
#define UTIL_EASYRANDOM_HPP_INCLUDED

/* Basically a wrapper API around <random>.
 * It's thread-safe, or at least it's supposed to be.
 * Call seed() in main().  */

#include <emu/util/unsigned.hpp>

namespace Util {

void seed();
uint8 random8();

} // namespace Util

#endif
