#pragma once

#include <cstdint>

namespace chess {

enum class Color : std::uint8_t {
    WHITE,
    BLACK,
};

inline constexpr Color invert(Color color) {
    return static_cast<Color>(static_cast<std::uint8_t>(color) ^ 1);
}

}  // namespace chess
