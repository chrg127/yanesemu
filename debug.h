// defined independent of DEBUG macro
#define error(...) do { std::fprintf(stderr, "error: " __VA_ARGS__); } while (0)
#define warning(...) do { std::fprintf(stderr, "warning: " __VA_ARGS__); } while (0)

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

