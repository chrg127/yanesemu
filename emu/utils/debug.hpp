/*
 * debug.h: debugging utilities: a collection of macros and inline functions to
 * be used for debugging and error messages.
 * #define DEBUG to use the macros. error message functions need not to define
 * it.
 */
#ifndef DEBUG_HPP_INCLUDED
#define DEBUG_HPP_INCLUDED

#include <cstdio>
#include <cstdarg>

inline void
#if defined(__GNUC__) || defined(__MINGW32__) || defined (__MINGW64__)
    __attribute__((format(printf, 1, 2)))
#endif
error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
}

inline void
#if defined(__GNUC__) || defined(__MINGW32__) || defined (__MINGW64__)
    __attribute__((format(printf, 1, 2)))
#endif
warning(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "warning: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
}

#ifdef DEBUG
#include <cstdio>
#include <cassert>
#define DBGPRINT(str)       do { std::fprintf(stderr, str); } while (0)
#define DBGPRINTF(fmt, ...) do { std::fprintf(stderr, fmt, __VA_ARGS__); } while (0)
#define GPRINTHEX8(val)     do { std::printf("%02X", val); } while (0)
#define DBGPRINTHEX16(val)  do { std::printf("%04X", val); } while (0)
#else
#define DBGPRINT(str)       ;
#define DBGPRINTF(fmt, ...) ;
#define DBGPRINTHEX8(val)   ;
#define DBGPRINTHEX16(val)  ;
#endif

#endif
