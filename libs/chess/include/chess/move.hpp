#pragma once

#include <cstdint>

#include "chess/piece.hpp"
#include "chess/square.hpp"

namespace chess {

enum class MoveType : std::uint8_t {
    NULL_MOVE,
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
    // constructor
    Move() noexcept :
            Move(
                Square::NO_SQ,
                Square::NO_SQ,
                MoveType::NULL_MOVE,
                Piece::NONE,
                Piece::NONE
            ) {}

    // new move creation methods
    static Move null() noexcept {
        return Move();
    }

    static Move quiet(Square from_square, Square to_square) noexcept {
        return Move(
            from_square,
            to_square,
            MoveType::QUIET,
            Piece::NONE,
            Piece::NONE
        );
    }

    static Move capture(
        Square from_square,
        Square to_square,
        Piece captured_piece
    ) noexcept {
        return Move(
            from_square,
            to_square,
            MoveType::CAPTURE,
            captured_piece,
            Piece::NONE
        );
    }

    static Move double_pawn_push(
        Square from_square,
        Square to_square
    ) noexcept {
        return Move(
            from_square,
            to_square,
            MoveType::DOUBLE_PAWN_PUSH,
            Piece::NONE,
            Piece::NONE
        );
    }

    static Move en_passant(Square from_square, Square to_square) noexcept {
        return Move(
            from_square,
            to_square,
            MoveType::EN_PASSANT,
            Piece::NONE,
            Piece::NONE
        );
    }

    static Move promotion(
        Square from_square,
        Square to_square,
        Piece promotion_piece
    ) noexcept {
        return Move(
            from_square,
            to_square,
            MoveType::PROMOTION,
            Piece::NONE,
            promotion_piece
        );
    }

    static Move promotion_capture(
        Square from_square,
        Square to_square,
        Piece captured_piece,
        Piece promotion_piece
    ) noexcept {
        return Move(
            from_square,
            to_square,
            MoveType::PROMOTION_CAPTURE,
            captured_piece,
            promotion_piece
        );
    }

    static Move castle_kingside(Square from_square, Square to_square) noexcept {
        return Move(
            from_square,
            to_square,
            MoveType::CASTLE_KINGSIDE,
            Piece::NONE,
            Piece::NONE
        );
    }

    static Move castle_queenside(
        Square from_square,
        Square to_square
    ) noexcept {
        return Move(
            from_square,
            to_square,
            MoveType::CASTLE_QUEENSIDE,
            Piece::NONE,
            Piece::NONE
        );
    }

    // getters
    Square get_from_square() const noexcept {
        return from_square_;
    }

    Square get_to_square() const noexcept {
        return to_square_;
    }

    MoveType get_type() const noexcept {
        return type_;
    }

    Piece get_captured_piece() const noexcept {
        return captured_piece_;
    }

    Piece get_promotion_piece() const noexcept {
        return promotion_piece_;
    }

private:
    Move(
        Square from_square,
        Square to_square,
        MoveType move_type,
        Piece captured_piece,
        Piece promotion_piece
    ) noexcept :
            from_square_(from_square),
            to_square_(to_square),
            type_(move_type),
            captured_piece_(captured_piece),
            promotion_piece_(promotion_piece) {}

    Square from_square_;
    Square to_square_;
    MoveType type_;
    Piece captured_piece_;
    Piece promotion_piece_;
};

}  // namespace chess
