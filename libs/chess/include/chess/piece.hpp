#pragma once

#include <cstdint>

#include "chess/color.hpp"

namespace chess {

enum class PieceType : std::uint8_t {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
};

namespace internal {

    constexpr std::uint8_t PIECE_COLOR_SHIFT = 4;
    constexpr std::uint8_t PIECE_TYPE_MASK = 0x0F;

    constexpr std::uint8_t get_piece_encoded_value(
        Color piece_color,
        PieceType piece_type
    ) noexcept {
        return (static_cast<std::uint8_t>(piece_color) << PIECE_COLOR_SHIFT) |
               static_cast<std::uint8_t>(piece_type);
    }

}  // namespace internal

enum class Piece : std::uint8_t {
    WHITE_PAWN =
        internal::get_piece_encoded_value(Color::WHITE, PieceType::PAWN),
    WHITE_KNIGHT =
        internal::get_piece_encoded_value(Color::WHITE, PieceType::KNIGHT),
    WHITE_BISHOP =
        internal::get_piece_encoded_value(Color::WHITE, PieceType::BISHOP),
    WHITE_ROOK =
        internal::get_piece_encoded_value(Color::WHITE, PieceType::ROOK),
    WHITE_QUEEN =
        internal::get_piece_encoded_value(Color::WHITE, PieceType::QUEEN),
    WHITE_KING =
        internal::get_piece_encoded_value(Color::WHITE, PieceType::KING),
    BLACK_PAWN =
        internal::get_piece_encoded_value(Color::BLACK, PieceType::PAWN),
    BLACK_KNIGHT =
        internal::get_piece_encoded_value(Color::BLACK, PieceType::KNIGHT),
    BLACK_BISHOP =
        internal::get_piece_encoded_value(Color::BLACK, PieceType::BISHOP),
    BLACK_ROOK =
        internal::get_piece_encoded_value(Color::BLACK, PieceType::ROOK),
    BLACK_QUEEN =
        internal::get_piece_encoded_value(Color::BLACK, PieceType::QUEEN),
    BLACK_KING =
        internal::get_piece_encoded_value(Color::BLACK, PieceType::KING),
};

inline constexpr Piece get_piece(
    Color piece_color,
    PieceType piece_type
) noexcept {
    return static_cast<Piece>(
        internal::get_piece_encoded_value(piece_color, piece_type)
    );
}

inline constexpr Color get_piece_color(Piece piece) noexcept {
    return static_cast<Color>(
        static_cast<std::uint8_t>(piece) >> internal::PIECE_COLOR_SHIFT
    );
}

inline constexpr PieceType get_piece_type(Piece piece) noexcept {
    return static_cast<PieceType>(
        static_cast<std::uint8_t>(piece) & internal::PIECE_TYPE_MASK
    );
}

}  // namespace chess
