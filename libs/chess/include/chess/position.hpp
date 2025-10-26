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

    static Position standard();
    static Position from_fen(std::string_view fen_string);

private:
    Position() noexcept;

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
