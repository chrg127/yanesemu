#include "conf.hpp"

#include <filesystem>
#include "stringops.hpp"
#include "debug.hpp"
#include "paths.hpp"

namespace Util {

static ConfValue parse_value(const std::string &s)
{
    if      (s == "true")                return true;
    else if (s == "false")               return false;
    else if (auto num = strconv(s); num) return num.value();
    else                                 return s;
}

ParseConfResult parse_conf(File &conf_file, ValidConfValues valid_settings)
{
    ParseConfResult res;

    struct Line {
        std::string str;
        std::size_t pos;
    };
    std::unordered_map<std::string, Line> kvmap;

    std::size_t lineno = 0;
    for (std::string line; conf_file.getline(line); lineno++) {
        // is this a comment?
        if (line[0] == '#' || line == "")
            continue;
        auto vs = strsplit(line, '=');
        if (vs.size() == 2) {
            kvmap[vs[0]] = {
                .str = vs[1],
                .pos = lineno
            };
        } else {
            res.errors.push_back({
                .line = line,
                .key  = "",
                .pos  = lineno,
                .kind = ConfValueError::Kind::INVLINE
            });
        }
    }

    for (const auto &valid_st : valid_settings) {
        auto it = kvmap.find(valid_st.key);
        if (it == kvmap.end()) {
            warning("key {} not found, using default\n", valid_st.key);
            res.st_map[valid_st.key] = valid_st.def;
            continue;
        }
        ConfValue setting = parse_value(it->second.str);
        if (setting.index() != valid_st.type) {
            res.errors.push_back({
                .line = valid_st.key + "=" + it->second.str,
                .key  = valid_st.key,
                .pos  = it->second.pos,
                .kind = ConfValueError::Kind::INVVAL
            });
            res.st_map[valid_st.key] = valid_st.def;
            continue;
        }
        res.st_map[valid_st.key] = setting;
    }
    return res;
}

void print_conf_errors(const std::vector<ConfValueError> &errors)
{
    for (const auto &err : errors) {
        fmt::print(stderr, "error at line {}: \n", err.pos);
        switch (err.kind) {
        case ConfValueError::Kind::INVVAL:
            fmt::print(stderr, "invalid value for key {}, using default value ({})\n", err.key, err.line);
            break;
        case ConfValueError::Kind::INVLINE:
            fmt::print(stderr, "invalid line ({})\n", err.line);
            break;
        }
    }
}

File open_conf_file(const std::string &dirname, const std::string &filename)
{
    File file;

    if (file.open(Path::user_settings() + dirname + "/" + filename, Access::MODIFY))
        return file;
    if (file.open(Path::user_home() + "." + dirname + "/" + filename, Access::MODIFY))
        return file;

    // couldn't find file, create a new directory and file.
    auto test_dir = [&](const std::string &dirpath)
    {
        if (std::filesystem::create_directory(dirpath)) {
            if (file.open(dirpath + filename, Access::WRITE)) {
                file.reopen(Access::MODIFY);
                return true;
            }
            error("can't create file {} in directory {}: ", filename, dirpath);
            std::perror("");
        } else {
            error("can't create directory {}\n", dirpath);
            std::perror("");
        }
        return false;
    };

    if (test_dir(Path::user_settings() + dirname + "/"))
        return file;
    if (test_dir(Path::user_home() + "." + dirname + "/"))
        return file;
    return File();
}

} // namespace Util

