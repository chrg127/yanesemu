#pragma once

#include <mutex>

namespace util {

template <typename T>
class Locked {
    T resource{};
    std::mutex mut;

public:
    Locked() = default;
    explicit Locked(T &&t) : resource(t) { }

    template <typename U>
    U access(auto &&fn)
    {
        std::lock_guard<std::mutex> lock(mut);
        return fn(resource);
    }

    void access(auto &&fn)
    {
        std::lock_guard<std::mutex> lock(mut);
        fn(resource);
    }
};

} // namespace util
