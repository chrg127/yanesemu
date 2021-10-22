#pragma once

#include <cstdlib>
#include <string_view>
#include <fmt/core.h>

constexpr inline void error(std::string_view fmtstr, auto&&... args)
{
    fmt::print(stderr, "error: ");
    // fmt::print(stderr, fmt::runtime(fmtstr), args...);
    fmt::print(stderr, fmtstr, args...);
}

constexpr inline void warning(std::string_view fmtstr, auto&&... args)
{
    fmt::print(stderr, "warning: ");
    // fmt::print(stderr, fmt::runtime(fmtstr), args...);
    fmt::print(stderr, fmtstr, args...);
}

[[noreturn]] inline void panic(std::string_view fmtstr, auto&&... args)
{
    fmt::print(stderr, "panic: ");
    // fmt::print(stderr, fmt::runtime(fmtstr), args...);
    fmt::print(stderr, fmtstr, args...);
    std::exit(1);
}

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