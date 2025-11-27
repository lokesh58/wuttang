#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

#include "chess/castling_rights.hpp"
#include "chess/color.hpp"
#include "chess/move.hpp"
#include "chess/move_list.hpp"
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
    Color get_side_to_move() const noexcept;
    bool has_castling_right(CastlingRights castling_right) const noexcept;
    bool has_kingside_castling_rights(Color color) const noexcept;
    bool has_queenside_castling_rights(Color color) const noexcept;
    std::optional<Square> get_en_passant_square() const noexcept;
    std::uint8_t get_halfmove_clock() const noexcept;
    std::string get_fen() const noexcept;

    void make_move(const Move& move);
    void undo_last_move();

    MoveList generate_legal_moves() const;

private:
    static constexpr std::size_t BOARD_SIZE = 64;
    using Board = std::array<std::optional<Piece>, BOARD_SIZE>;

    Position() noexcept;

    static bool is_valid_fen(std::string_view fen_string) noexcept;
    static Position from_valid_fen(std::string_view fen_string) noexcept;

    void set_piece_at(Square square, std::optional<Piece> piece) noexcept;

    bool is_valid_move(const Move& move) const noexcept;
    void add_piece(Square square, Piece piece) noexcept;
    void remove_piece(Square square) noexcept;
    void move_piece(Square from_square, Square to_square) noexcept;

    void generate_pawn_moves(MoveList& moves, Square start_square) const;
    void generate_knight_moves(MoveList& moves, Square start_square) const;
    void generate_bishop_moves(MoveList& moves, Square start_square) const;
    void generate_rook_moves(MoveList& moves, Square start_square) const;
    void generate_queen_moves(MoveList& moves, Square start_square) const;
    void generate_king_moves(MoveList& moves, Square start_square) const;

    MoveList generate_pseudo_legal_moves() const;
    bool is_move_legal(const Move& move) const;
    bool is_king_in_check(Color king_color) const;
    std::optional<Square> find_king(Color king_color) const;
    bool is_square_attacked(Square s, Color attacker_color) const;

    static bool is_square_attacked_internal(const Board& board, Square s, Color attacker_color);
    static std::optional<Square> find_king_internal(const Board& board, Color king_color);
    static bool is_king_in_check_internal(const Board& board, Color king_color);

    struct History {
        Move move;
        std::optional<Square> en_passant_square;
        CastlingRights castling_rights;
        std::uint8_t halfmove_clock;
    };

    Board board_;
    Color side_to_move_;
    std::optional<Square> en_passant_square_;
    CastlingRights castling_rights_;
    std::uint8_t halfmove_clock_;
    std::vector<History> history_;
};

}  // namespace chess
