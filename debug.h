#ifdef DEBUG
#define DBGPRINT(str)       std::printf(str)
#define DBGPRINTF(fmt, ...) std::printf(fmt, __VA_ARGS__)
#define DBGPRINTHEX8(val)   std::printf("%02X", val)
#define DBGPRINTHEX16(val)  std::printf("%04X", val)
#else
#define DBGPRINT(str)       ;
#define DBGPRINTF(fmt, ...) ;
#define DBGPRINTHEX8(val)   ;
#define DBGPRINTHEX16(val)  ;
#endif

