#pragma once

#include <cstdint>
#include <optional>

#include "chess/piece.hpp"
#include "chess/square.hpp"

namespace chess {

enum class MoveType : std::uint8_t {
    QUIET,
    CAPTURE,
    DOUBLE_PAWN_PUSH,
    EN_PASSANT,
    PROMOTION,
    PROMOTION_CAPTURE,
    CASTLE_KINGSIDE,
    CASTLE_QUEENSIDE,
};

class Move {
public:
    static Move quiet(Square from_square, Square to_square) noexcept;
    static Move capture(
        Square from_square,
        Square to_square,
        Piece captured_piece
    ) noexcept;
    static Move double_pawn_push(Square from_square, Square to_square) noexcept;
    static Move en_passant(Square from_square, Square to_square) noexcept;
    static Move promotion(
        Square from_square,
        Square to_square,
        Piece promotion_piece
    ) noexcept;
    static Move promotion_capture(
        Square from_square,
        Square to_square,
        Piece captured_piece,
        Piece promotion_piece
    ) noexcept;
    static Move castle_kingside(Square from_square, Square to_square) noexcept;
    static Move castle_queenside(Square from_square, Square to_square) noexcept;

    Square get_from_square() const noexcept;
    Square get_to_square() const noexcept;
    MoveType get_type() const noexcept;
    std::optional<Piece> get_captured_piece() const noexcept;
    std::optional<Piece> get_promotion_piece() const noexcept;

private:
    Move(
        Square from_square,
        Square to_square,
        MoveType move_type,
        std::optional<Piece> captured_piece,
        std::optional<Piece> promotion_piece
    ) noexcept;

    Square from_square_;
    Square to_square_;
    MoveType type_;
    std::optional<Piece> captured_piece_;
    std::optional<Piece> promotion_piece_;
};

}  // namespace chess
