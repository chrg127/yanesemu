#ifndef UTIL_DEBUG_HPP_INCLUDED
#define UTIL_DEBUG_HPP_INCLUDED

/* This is a collection of macros and inline functions to be used for debugging and error messages.
 * #define DEBUG to use the macros. Error message functions need not to define it.
 *
 * - error(): prints a message to stderr with leading 'error: '.
 * - warning(): same as above but with warning instead.
 * - panic(): same as above, but with panic instead, and immediately exits.
 * - dbgprint(): same as above but with leading file and line instead. The
 *   message is seen only with DEBUG defined.
 */

#include <fmt/core.h>

template <typename... T>
inline void error(std::string &&fmt, T&&... args)
{
    fmt::print(stderr, "error: ");
    fmt::print(stderr, fmt, args...);
}

template <typename... T>
inline void warning(std::string &&fmt, T&&... args)
{
    fmt::print(stderr, "warning: ");
    fmt::print(stderr, fmt, args...);
}

template <typename... T>
[[noreturn]] inline void panic(std::string &&fmt, T&&... args)
{
    fmt::print(stderr, "panic: ");
    fmt::print(stderr, fmt, args...);
    std::exit(1);
}

#ifdef DEBUG

template <typename... T>
inline void _dbgprint_detail(const char *file, int line, std::string &&fmt, T&&... args)
{
    fmt::print(stderr, "{}:{} ", file, line);
    fmt::print(stderr, fmt, args...);
}
#define dbgprint(fmt, ...) do { dbgprint_detail(__FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__); } while (0)

#else
#define dbgprint(...) ;
#endif

#endif
