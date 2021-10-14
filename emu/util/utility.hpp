#pragma once

#include <cstring>
#include <functional>
#include <optional>

namespace util {

/* Lookup value for a constant map, returning defval if not found.
 * const maps have the problem that map[key] will never work, since
 * map::operator[] might modify the map. One is stuck using map::find()
 * and iterators.
 * This function makes find() a little less awkward. */
template <typename M>
const typename M::mapped_type &map_lookup_withdef(const M &m,
    const typename M::key_type &key, const typename M::mapped_type &defval)
{
    auto it = m.find(key);
    return it == m.end() ? defval : it->second;
}

template <typename M>
std::optional<typename M::mapped_type> map_lookup(const M &m, const typename M::key_type &key)
{
    auto it = m.find(key);
    if (it == m.end())
        return std::nullopt;
    return it->second;
}

/* Makes an std::function from a member function without having
 * to throw readability away.
 * Example:
 *      struct MyStruct { uint8 read(uint16); };
 *      mymemfn = member_fn(&my_struct_instance, &MyStruct::read);
 *      uint8 res = mymemfn(0);
 */
template <typename T, typename R, typename... Args>
std::function<R(Args...)> member_fn(T *obj, R (T::*fn)(Args...))
{
    return [=](Args&&... args) -> R { return (obj->*fn)(args...); };
}

template <typename T>
concept ContainerType = requires(T t) {
    t.data();
};

inline std::string_view system_error_string()
{
    return std::strerror(errno);
}

} // namespace util
