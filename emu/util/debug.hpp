#ifndef UTIL_DEBUG_HPP_INCLUDED
#define UTIL_DEBUG_HPP_INCLUDED

/* This is a collection of macros and inline functions to
 * be used for debugging and error messages.
 * #define DEBUG to use the macros. Error message functions need not to define it. */

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

template <typename... T>
#if defined(__GNUC__) || defined(__GNUG__)
__attribute__((noreturn))
#endif
inline void panic(std::string &&fmt, T... args)
{
    fmt::print(stderr, "panic: ");
    fmt::print(stderr, fmt, args...);
    std::exit(1);
}

#ifdef DEBUG
template <typename... T>
inline void dbgprint(const char *file, int line, std::string &&fmt, T... args)
{
    fmt::print(stderr, "{}:{} ", file, line);
    fmt::print(stderr, fmt, args...);
}
#else
#define dbgprint(...) ;
#endif

#endif
