#pragma once

#include <cassert>
#include <optional>

namespace utils {

template<typename T>
inline constexpr T unwrap(std::optional<T> opt) noexcept {
    assert(opt.has_value());
    return *opt;
}

}  // namespace utils
