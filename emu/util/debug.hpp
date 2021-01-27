/* debug.h: debugging utilities: a collection of macros and inline functions to
 * be used for debugging and error messages.
 * #define DEBUG to use the macros. error message functions need not to define
 * it. */
#ifndef UTILS_DEBUG_HPP_INCLUDED
#define UTILS_DEBUG_HPP_INCLUDED

#include <cstdio>
#include <cstdarg>

#define error(fmt, ...)   do { fprintf(stderr, "error: " fmt __VA_OPT__(, __VA_ARGS__)); } while (0)
#define warning(fmt, ...) do { fprintf(stderr, "warning: " fmt __VA_OPT__(, __VA_ARGS__)); } while (0)

#ifdef DEBUG
#include <cassert>
#define DBGPRINT(str)       do { std::fprintf(stderr, str); } while (0)
#define DBGPRINTF(fmt, ...) do { std::fprintf(stderr, fmt, __VA_ARGS__); } while (0)
#define GPRINTHEX8(val)     do { std::printf("%02X", val); } while (0)
#define DBGPRINTHEX16(val)  do { std::printf("%04X", val); } while (0)
#define DBGPUTC(c)          ;
//do { std::putchar(c); } while (0)
#else
#define DBGPRINT(str)       ;
#define DBGPRINTF(fmt, ...) ;
#define DBGPRINTHEX8(val)   ;
#define DBGPRINTHEX16(val)  ;
#define DBGPUTC(c)          ;
#endif

#endif
