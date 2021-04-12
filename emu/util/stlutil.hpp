#ifndef UTIL_STLUTIL_HPP_INCLUDED
#define UTIL_STLUTIL_HPP_INCLUDED

#include <optional>
#include <unordered_map>
#include <vector>

/* STL collections utilities.
 * These were made for fixing some dumb problems in standard collections.
 * */
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

} // namespace Util

#endif
