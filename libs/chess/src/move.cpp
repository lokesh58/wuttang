#include "chess/move.hpp"

#include <optional>

#include "chess/piece.hpp"
#include "chess/square.hpp"

namespace chess {

Move::Move() noexcept:
        Move(
            Square::A1,
            Square::A1,
            MoveType::NULL_MOVE,
            std::nullopt,
            std::nullopt
        ) {}

Move Move::null() noexcept {
    return Move();
}

Move::Move(
    Square from_square,
    Square to_square,
    MoveType move_type,
    std::optional<Piece> captured_piece,
    std::optional<Piece> promotion_piece
) noexcept:
        from_square_(from_square),
        to_square_(to_square),
        type_(move_type),
        captured_piece_(captured_piece),
        promotion_piece_(promotion_piece) {}

Move Move::quiet(Square from_square, Square to_square) noexcept {
    return Move(
        from_square,
        to_square,
        MoveType::QUIET,
        std::nullopt,
        std::nullopt
    );
}

Move Move::capture(
    Square from_square,
    Square to_square,
    Piece captured_piece
) noexcept {
    return Move(
        from_square,
        to_square,
        MoveType::CAPTURE,
        captured_piece,
        std::nullopt
    );
}

Move Move::double_pawn_push(Square from_square, Square to_square) noexcept {
    return Move(
        from_square,
        to_square,
        MoveType::DOUBLE_PAWN_PUSH,
        std::nullopt,
        std::nullopt
    );
}

Move Move::en_passant(Square from_square, Square to_square) noexcept {
    return Move(
        from_square,
        to_square,
        MoveType::EN_PASSANT,
        std::nullopt,
        std::nullopt
    );
}

Move Move::promotion(
    Square from_square,
    Square to_square,
    Piece promotion_piece
) noexcept {
    return Move(
        from_square,
        to_square,
        MoveType::PROMOTION,
        std::nullopt,
        promotion_piece
    );
}

Move Move::promotion_capture(
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

Move Move::castle_kingside(Square from_square, Square to_square) noexcept {
    return Move(
        from_square,
        to_square,
        MoveType::CASTLE_KINGSIDE,
        std::nullopt,
        std::nullopt
    );
}

Move Move::castle_queenside(Square from_square, Square to_square) noexcept {
    return Move(
        from_square,
        to_square,
        MoveType::CASTLE_QUEENSIDE,
        std::nullopt,
        std::nullopt
    );
}

Square Move::get_from_square() const noexcept {
    return from_square_;
}

Square Move::get_to_square() const noexcept {
    return to_square_;
}

MoveType Move::get_type() const noexcept {
    return type_;
}

std::optional<Piece> Move::get_captured_piece() const noexcept {
    return captured_piece_;
}

std::optional<Piece> Move::get_promotion_piece() const noexcept {
    return promotion_piece_;
}

}  // namespace chess
