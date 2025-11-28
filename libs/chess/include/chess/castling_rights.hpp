#pragma once

#include <cstdint>
#include <utils/enum_flags.hpp>

namespace chess {

enum class CastlingRights : std::uint8_t {
    NONE = 0,
    WHITE_KINGSIDE = 1 << 0,
    WHITE_QUEENSIDE = 1 << 1,
    WHITE_ALL = WHITE_KINGSIDE | WHITE_QUEENSIDE,
    BLACK_KINGSIDE = 1 << 2,
    BLACK_QUEENSIDE = 1 << 3,
    BLACK_ALL = BLACK_KINGSIDE | BLACK_QUEENSIDE,
    ALL = WHITE_ALL | BLACK_ALL,
};

}  // namespace chess

namespace utils {

template<>
struct enable_bitmask_operators<chess::CastlingRights> {
    static constexpr bool value = true;
};

}  // namespace utils
