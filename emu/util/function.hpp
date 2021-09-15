#pragma once

#include <utility>
#ifdef DEBUG
#include <exception>
#endif

namespace Util {

template <typename T>
struct Function;

template <typename R, typename... P>
class Function<R(P...)> {
    struct Base {
        virtual R operator()(P... ps) const = 0;
        virtual ~Base() = default;
    };

    struct FnPtr : Base {
        R (*fn)(P...);

        FnPtr(R (*ptr)(P...)) : fn(ptr) { }
        R operator()(P... ps) const { return fn(std::forward<P>(ps)...); }
    };

    template <typename M>
    struct Member : Base {
        R (M::*fn)(P...);
        M *obj;

        Member(M *optr, R (M::*ptr)(P...)) : fn(ptr), obj(optr) { }
        R operator()(P... ps) const { return (obj->*fn)(std::forward<P>(ps)...); }
    };

    template <typename L>
    struct Lambda : Base {
        mutable L lambda;

        Lambda(const L &l) : lambda(l) { }
        R operator()(P... ps) const { return lambda(std::forward<P>(ps)...); }
    };

    Base *closure = nullptr;

public:
    Function() = default;
    Function(const Function &f) = delete;
    Function(Function &&f) { operator=(std::move(f)); }
    Function & operator=(const Function &f) = delete;
    Function & operator=(Function &&f)
    {
        closure = f.closure;
        f.closure = nullptr;
        return *this;
    }

    ~Function() { if (closure) delete closure; }

                          Function(R (*fn)(P...))            { closure = new FnPtr(fn); }
    template <typename M> Function(M *obj, R (M::*fn)(P...)) { closure = new Member<M>(obj, fn); }
    template <typename L> Function(const L &l)               { closure = new Lambda<L>(l); }

    explicit operator bool() const { return closure != nullptr; }

    R operator()(P... ps) const
    {
#ifdef DEBUG
        if (!closure)
            throw std::runtime_error("Bad function call");
#endif
        return (*closure)(std::forward<P>(ps)...);
    }
};

} // namespace Util
