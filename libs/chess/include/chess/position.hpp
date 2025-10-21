#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

#include "chess/castling_rights.hpp"
#include "chess/color.hpp"
#include "chess/move.hpp"
#include "chess/piece.hpp"
#include "chess/square.hpp"

namespace chess {

class Position {
public:
    Position();

private:
    static constexpr std::size_t BOARD_SIZE = 64;
    using Board = std::array<std::optional<Piece>, BOARD_SIZE>;

    struct History {
        Move move;
        std::optional<Square> en_passant_square;
        CastlingRights castling_rights;
    };

    Board board_;
    Color side_to_move_;
    std::optional<Square> en_passant_square_;
    CastlingRights castling_rights_;
    std::uint8_t half_move_counter_;
    std::vector<History> history_;
};

}  // namespace chess
