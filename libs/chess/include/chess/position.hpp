#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
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

    std::string get_fen() const noexcept;

    Piece get_piece_at(Square square) const noexcept {
        return board_[static_cast<std::size_t>(square)];
    }
    Color get_side_to_move() const noexcept {
        return side_to_move_;
    }
    bool has_castling_right(CastlingRights rights_to_check) const noexcept {
        return (rights_to_check & castling_rights_) == rights_to_check;
    }
    bool has_kingside_castling_rights(Color color) const noexcept {
        auto rights_to_check = color == Color::WHITE
                                   ? CastlingRights::WHITE_KINGSIDE
                                   : CastlingRights::BLACK_KINGSIDE;
        return has_castling_right(rights_to_check);
    }
    bool has_queenside_castling_rights(Color color) const noexcept {
        auto rights_to_check = color == Color::WHITE
                                   ? CastlingRights::WHITE_QUEENSIDE
                                   : CastlingRights::BLACK_QUEENSIDE;
        return has_castling_right(rights_to_check);
    }
    Square get_en_passant_square() const noexcept {
        return en_passant_square_;
    }
    std::uint8_t get_halfmove_clock() const noexcept {
        return halfmove_clock_;
    }
    std::uint16_t get_fullmove_number() const noexcept {
        const std::uint16_t plies = history_.size();
        const bool is_odd_plies = (plies % 2 != 0);
        const Color initial_side_to_move =
            is_odd_plies ? invert(side_to_move_) : side_to_move_;
        const std::uint16_t ply_offset =
            initial_side_to_move == Color::BLACK ? 1 : 0;
        const std::uint16_t full_moves_played = (plies + ply_offset) / 2;

        return initial_fullmove_number_ + full_moves_played;
    }

    void make_move(const Move& move);
    void undo_last_move();

private:
    static constexpr std::size_t BOARD_SIZE = 64;
    using Board = std::array<Piece, BOARD_SIZE>;

    Position() noexcept;

    static bool is_valid_fen(std::string_view fen_string) noexcept;
    static Position from_valid_fen(std::string_view fen_string) noexcept;

    void set_piece_at(Square square, Piece piece) noexcept {
        board_[static_cast<std::size_t>(square)] = piece;
    }
    void add_castling_rights(CastlingRights rights_to_add) noexcept {
        castling_rights_ |= rights_to_add;
    }

    bool is_valid_move(const Move& move) const noexcept;
    void do_make_move(const Move& move) noexcept;
    void do_undo_last_move() noexcept;
    void add_piece(Square square, Piece piece) noexcept;
    void remove_piece(Square square) noexcept;
    void move_piece(Square from_square, Square to_square) noexcept;

    struct History {
        Move move;
        Square en_passant_square;
        CastlingRights castling_rights;
        std::uint8_t halfmove_clock;
    };

    Board board_;
    Color side_to_move_;
    Square en_passant_square_;
    CastlingRights castling_rights_;
    std::uint8_t halfmove_clock_;
    std::uint16_t initial_fullmove_number_;
    std::vector<History> history_;
};

}  // namespace chess
