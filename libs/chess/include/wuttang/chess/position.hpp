#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>
#include <wuttang/chess/bitboard.hpp>
#include <wuttang/chess/castling_rights.hpp>
#include <wuttang/chess/color.hpp>
#include <wuttang/chess/move.hpp>
#include <wuttang/chess/piece.hpp>
#include <wuttang/chess/square.hpp>

namespace wuttang::chess {

class Position {
    friend class MoveGenerator;

public:
    static constexpr std::string_view STANDARD_STARTING_FEN =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    static Position standard() noexcept;
    static Position from_fen(std::string_view fen_string);

    std::string get_fen() const noexcept;

    Piece get_piece_at(Square square) const noexcept {
        assert(static_cast<std::size_t>(square) < BOARD_SIZE);
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
    std::uint64_t get_hash() const noexcept {
        return hash_;
    }

    Bitboard get_occupancy() const noexcept {
        return color_bitboards_[0] | color_bitboards_[1];
    }
    Bitboard get_occupancy(Color color) const noexcept {
        assert(color != Color::NONE);
        return color_bitboards_[static_cast<std::size_t>(color)];
    }
    Bitboard get_bitboard(PieceType type) const noexcept {
        assert(type != PieceType::NONE);
        return piece_type_bitboards_[static_cast<std::size_t>(type)];
    }
    Bitboard get_bitboard(Color color, PieceType type) const noexcept {
        assert(color != Color::NONE);
        assert(type != PieceType::NONE);
        return color_bitboards_[static_cast<std::size_t>(color)] &
               piece_type_bitboards_[static_cast<std::size_t>(type)];
    }

    bool is_square_attacked(Square sq, Color attacker) const noexcept;

    void make_move(const Move& move) noexcept;
    void undo_last_move() noexcept;

private:
    static constexpr std::size_t BOARD_SIZE = 64;
    static constexpr File KING_FILE = File::FILE_E;
    static constexpr File KINGSIDE_ROOK_FILE = File::FILE_H;
    static constexpr File QUEENSIDE_ROOK_FILE = File::FILE_A;
    static constexpr std::array<CastlingRights, BOARD_SIZE>
        CASTLING_RIGHTS_MASK = [] {
            std::array<CastlingRights, BOARD_SIZE> mask;
            mask.fill(CastlingRights::ALL);
            const std::array<std::tuple<File, Rank, CastlingRights>, 6>
                overrides{{
                    {KING_FILE, Rank::RANK_1, CastlingRights::WHITE_ALL},
                    {KINGSIDE_ROOK_FILE,
                     Rank::RANK_1,
                     CastlingRights::WHITE_KINGSIDE},
                    {QUEENSIDE_ROOK_FILE,
                     Rank::RANK_1,
                     CastlingRights::WHITE_QUEENSIDE},
                    {KING_FILE, Rank::RANK_8, CastlingRights::BLACK_ALL},
                    {KINGSIDE_ROOK_FILE,
                     Rank::RANK_8,
                     CastlingRights::BLACK_KINGSIDE},
                    {QUEENSIDE_ROOK_FILE,
                     Rank::RANK_8,
                     CastlingRights::BLACK_QUEENSIDE},
                }};
            for (const auto& [file, rank, rights_to_mask] : overrides) {
                const auto square = get_square_from_file_rank(file, rank);
                mask[static_cast<std::size_t>(square)] = ~rights_to_mask;
            }
            return mask;
        }();

    Position() noexcept;

    static bool is_valid_fen(std::string_view fen_string) noexcept;
    static Position from_valid_fen(std::string_view fen_string);

    void set_piece_at(Square square, Piece piece) noexcept {
        assert(static_cast<std::size_t>(square) < BOARD_SIZE);
        board_[static_cast<std::size_t>(square)] = piece;
    }
    void add_castling_rights(CastlingRights rights_to_add) noexcept {
        castling_rights_ |= rights_to_add;
    }
    Bitboard& bitboard_of(Color color) noexcept {
        return color_bitboards_[static_cast<std::size_t>(color)];
    }
    Bitboard& bitboard_of(PieceType type) noexcept {
        return piece_type_bitboards_[static_cast<std::size_t>(type)];
    }

    bool is_well_formed_move(const Move& move) const noexcept;
    void make_well_formed_move(const Move& move) noexcept;
    void undo_last_move_with_non_empty_history() noexcept;
    void make_quiet_move(const Move& move) noexcept;
    void undo_quiet_move(const Move& move) noexcept;
    void make_capture_move(const Move& move) noexcept;
    void undo_capture_move(const Move& move) noexcept;
    void make_double_pawn_push_move(const Move& move) noexcept;
    void undo_double_pawn_push_move(const Move& move) noexcept;
    void make_en_passant_move(const Move& move) noexcept;
    void undo_en_passant_move(const Move& move) noexcept;
    void make_promotion_move(const Move& move) noexcept;
    void undo_promotion_move(const Move& move) noexcept;
    void make_promotion_capture_move(const Move& move) noexcept;
    void undo_promotion_capture_move(const Move& move) noexcept;
    void make_castle_kingside_move(const Move& move) noexcept;
    void undo_castle_kingside_move(const Move& move) noexcept;
    void make_castle_queenside_move(const Move& move) noexcept;
    void undo_castle_queenside_move(const Move& move) noexcept;
    void make_null_move() noexcept;
    void undo_null_move() noexcept;
    void add_piece(Square square, Piece piece) noexcept;
    void remove_piece(Square square) noexcept;
    void move_piece(Square from_square, Square to_square) noexcept;

    bool is_square_attacked(
        Square sq,
        Color attacker,
        Bitboard occupancy,
        Bitboard attackers_mask
    ) const noexcept;

    struct History {
        Move move;
        Square en_passant_square;
        CastlingRights castling_rights;
        std::uint8_t halfmove_clock;
        std::uint64_t hash;
    };

    std::array<Piece, BOARD_SIZE> board_;
    Color side_to_move_;
    Square en_passant_square_;
    CastlingRights castling_rights_;
    std::uint8_t halfmove_clock_;
    std::uint16_t initial_fullmove_number_;
    std::vector<History> history_;
    std::uint64_t hash_;
    std::array<Bitboard, 2> color_bitboards_;
    std::array<Bitboard, 7> piece_type_bitboards_;
};

}  // namespace wuttang::chess
