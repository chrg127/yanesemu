#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

/* STL collections utilities.
 * These were made for fixing some dumb problems in standard collections. */

namespace Util {

/* Lookup value for a constant map, returning defval if not found.
 * const maps have the problem that map[key] will never work, since
 * map::operator[] might modify the map. One is stuck using map::find()
 * and iterators.
 * This function makes find() a little less awkward. */
template <typename K, typename V>
const V &map_lookup_withdef(const std::unordered_map<K, V> &m,
                                   const K &key, const V &defval)
{
    auto it = m.find(key);
    return it == m.end() ? defval : it->second;
}

template <typename K, typename V>
std::optional<V> map_lookup(const std::unordered_map<K, V> &m, const K &key)
{
    auto it = m.find(key);
    if (it == m.end())
       return std::nullopt;
    return it->second;
}

/* Makes a visitor object suitable for std::visit().
 * Example:
 *     std::variant<int, std::string> var = "hello world";
 *     auto visitor = make_visitor(
 *         [](const std::string &str) { printf("%s\n", str.c_str()); }
 *         [](int n)                  { printf("%d\n", n); }
 *     );
 *     std::visit(visitor, var);
 */

template <typename... T>
struct Visitor : T... {
    using T::operator()...;

    Visitor(const T&... args) : T(args)...
    { }
};

template <typename... T>
Visitor<T...> make_visitor(T... lambdas)
{
    return Visitor<T...>(lambdas...);
}

} // namespace Util
