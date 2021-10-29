#pragma once

// a library for gdb-style command line applications

#include <functional>
#include <string>
#include <string_view>
#include <stdexcept>
#include <utility>
#include <span>
#include <fmt/core.h>

namespace util {

struct ParseError : std::runtime_error {
    using std::runtime_error::runtime_error;
    using std::runtime_error::what;
};

template <typename... P>
struct Command {
    std::string name;
    std::string abbrev;
    std::function<void(P...)> fn;
    Command(std::string_view n, std::string_view a, std::function<void(P...)> f) : name(n), abbrev(a), fn(f) { }
    Command(std::string_view n, std::string_view a, void (*f)(P...))             : name(n), abbrev(a), fn(f) { }
    Command(std::string_view n, std::string_view a, auto &&f)                    : name(n), abbrev(a), fn(f) { }
    template <typename T>
    Command(std::string_view n, std::string_view a, void (T::*f)(P...), T *t)    : name(n), abbrev(a)
    {
        fn = [t, f](P&&... p) { (t->*f)(p...); };
    }
};

template <typename T> struct TryConvert;

template <>
struct TryConvert<std::string_view> {
    static std::string_view convert(std::string_view str)
    {
        return str;
    }
};

namespace detail {

template <typename T>
auto try_convert(std::string_view str) {
    return TryConvert<T>::convert(str);
};

template<int N, typename... Ts>
using NthTypeOf = typename std::tuple_element<N, std::tuple<Ts...>>::type;

template <typename... P>
auto call_one(std::string_view name, std::span<std::string> args, bool &wrong_num_params, Command<P...> &cmd)
{
    auto call_forward = [&]<std::size_t... Is>(std::index_sequence<Is...> const&) {
        cmd.fn( detail::try_convert<NthTypeOf<Is, P...>>(std::string_view(args[Is]))... );
    };

    if (cmd.name != name && cmd.abbrev != name)
        return false;
    if (args.size() != sizeof...(P)) {
        wrong_num_params = true;
        return false;
    }
        // throw ParseError(fmt::format("command \"{}\" accepts {} arguments ({} passed)", name, sizeof...(P), args.size()));
    call_forward(std::index_sequence_for<P...>());
    return true;
};

} // namespace detail

void call_command(std::string_view name, std::span<std::string> args, auto &&invalid_fn, auto&&... commands)
{
    bool wrong_num_params = false;
    if (!(detail::call_one(name, args, wrong_num_params, commands) || ...))
        throw ParseError(invalid_fn(wrong_num_params ? 1 : 0, name, args.size()));
}

void call_command(std::span<std::string> args, auto &&invalid_fn, auto&&... commands)
{
    call_command(args[0], args.subspan(1, args.size()-1), invalid_fn, commands...);
}

} // namespace util
