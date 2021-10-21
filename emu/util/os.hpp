#pragma once

#if defined(_WIN32)
#   define PLATFORM_WINDOWS
#elif defined(_linux_) || defined(__linux__)
#   define PLATFORM_LINUX
#else
#   error "Unknown platform"
#endif