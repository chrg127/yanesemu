/* debug.h: debugging utilities: a collection of macros and inline functions to
 * be used for debugging and error messages.
 * #define DEBUG to use the macros. error message functions need not to define
 * it. */
#ifndef UTILS_DEBUG_HPP_INCLUDED
#define UTILS_DEBUG_HPP_INCLUDED

#include <fmt/core.h>

template <typename... T>
inline void error(std::string &&fmt, T... args)
{
    fmt::print(stderr, "error: ");
    fmt::print(stderr, fmt, args...);
}

template <typename... T>
inline void warning(std::string &&fmt, T... args)
{
    fmt::print(stderr, "warning: ");
    fmt::print(stderr, fmt, args...);
}

#ifdef DEBUG

template <typename... T>
inline void dbgprint(std::string &&fmt, T... args)
{
    fmt::print(stderr, "{}:{}\n", __FILE__, __LINE__);
    fmt::print(stderr, fmt, args...);
}
#define dbgputc(c) do { std::fputc(c, stderr); } while (0)

#else

#define dbgprint(...) ;
#define dbgputc(c) ;

#endif

#endif
