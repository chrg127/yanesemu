#ifndef PATHS_HPP_INCLUDED
#define PATHS_HPP_INCLUDED

#include <string>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

namespace Util {

namespace Path {

inline std::string user_home()
{
    return std::string(getpwuid(getuid())->pw_dir) + "/";
}

inline std::string user_settings()
{
    return user_home() + ".config/";
}

inline std::string user_data()
{
    return user_home() + ".local/share/";
}

inline std::string user_desktop()
{
    return user_home() + "Desktop/";
}

inline std::string root()
{
    return "/";
}

inline std::string shared_data()
{
    return "/usr/share/";
}

inline std::string tmp_dir()
{
    return "/tmp";
}

} // namespace Path

} // namespace Util

#endif

