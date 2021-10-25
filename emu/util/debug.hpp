#pragma once

#include <cstdlib>
#include <string_view>
#include <fmt/core.h>

#if FMT_VERSION >= 80001
#   define PRINT(str, args) fmt::print(stderr, fmt::runtime(str), args...)
#else
#   define PRINT(str, args) fmt::print(stderr, str, args...)
#endif

constexpr inline void error(std::string_view str, auto&&... args)
{
    fmt::print(stderr, "error: ");
    PRINT(str, args);
}

constexpr inline void warning(std::string_view str, auto&&... args)
{
    fmt::print(stderr, "warning: ");
    PRINT(str, args);
}

[[noreturn]] inline void panic(std::string_view str, auto&&... args)
{
    fmt::print(stderr, "panic: ");
    PRINT(str, args);
    std::exit(1);
}

#undef PRINT

#ifdef DEBUG

template <typename... T>
constexpr inline void _dbgprint_detail(const char *file, int line, std::string &&fmt, T&&... args)
{
    fmt::print(stderr, "{}:{} ", file, line);
    fmt::print(stderr, fmt, args...);
}
#define dbgprint(fmt, ...) do { dbgprint_detail(__FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__); } while (0)

#else
#define dbgprint(...) ;
#endif
