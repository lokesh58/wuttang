#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

#include "chess/castling_rights.hpp"
#include "chess/color.hpp"
#include "chess/move.hpp"
#include "chess/piece.hpp"
#include "chess/square.hpp"

namespace chess {

class Position {
public:
    static constexpr std::string_view STANDARD_STARTING_FEN =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    static Position standard() noexcept;
    static Position from_fen(std::string_view fen_string);

    std::optional<Piece> get_piece_at(Square square) const noexcept;

    void make_move(const Move& move);
    void undo_last_move();

private:
    Position() noexcept;

    static bool is_valid_fen(std::string_view fen_string) noexcept;
    static Position from_valid_fen(std::string_view fen_string) noexcept;

    static constexpr std::size_t BOARD_SIZE = 64;
    using Board = std::array<std::optional<Piece>, BOARD_SIZE>;

    struct History {
        Move move;
        std::optional<Square> en_passant_square;
        CastlingRights castling_rights;
        std::uint8_t half_move_clock;
    };

    Board board_;
    Color side_to_move_;
    std::optional<Square> en_passant_square_;
    CastlingRights castling_rights_;
    std::uint8_t half_move_clock_;
    std::vector<History> history_;
};

}  // namespace chess
