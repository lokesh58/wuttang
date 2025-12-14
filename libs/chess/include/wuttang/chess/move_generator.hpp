#pragma once

#include <cassert>
#include <cstdint>
#include <wuttang/chess/attacks.hpp>
#include <wuttang/chess/bitboard.hpp>
#include <wuttang/chess/color.hpp>
#include <wuttang/chess/move.hpp>
#include <wuttang/chess/move_list.hpp>
#include <wuttang/chess/piece.hpp>
#include <wuttang/chess/position.hpp>
#include <wuttang/chess/square.hpp>

namespace wuttang::chess {

enum class MoveGenType {
    ALL,       // All pseudo-legal moves
    LEGAL,     // All legal moves
    CAPTURES,  // Pseudo-legal captures
    QUIETS     // Pseudo-legal quiet moves
};

class MoveGenerator {
public:
    template<MoveGenType Type>
    static void generate(Position& position, MoveList& moves) noexcept {
        const Color us = position.get_side_to_move();
        const std::size_t start_index = moves.size();

        // 1. Generate Pseudo-Legal Moves directly into user list
        generate_pawn_moves<Type>(position, us, moves);
        generate_moves_for_piece<Type, PieceType::KNIGHT>(position, us, moves);
        generate_moves_for_piece<Type, PieceType::BISHOP>(position, us, moves);
        generate_moves_for_piece<Type, PieceType::ROOK>(position, us, moves);
        generate_moves_for_piece<Type, PieceType::QUEEN>(position, us, moves);
        generate_king_moves<Type>(position, us, moves);

        if constexpr (Type != MoveGenType::CAPTURES) {
            generate_castling_moves(position, us, moves);
        }

        // 2. Filter for Legality if requested
        if constexpr (Type == MoveGenType::LEGAL) {
            std::size_t i = start_index;
            while (i < moves.size()) {
                if (is_legal_move(position, moves[i])) {
                    ++i;
                } else {
                    // Swap with the last element and pop back
                    if (i != moves.size() - 1) {
                        moves[i] = moves[moves.size() - 1];
                    }
                    moves.pop_back();
                }
            }
        }
    }

    static bool is_legal_move(const Position& pos, const Move& move) noexcept {
        assert(pos.is_well_formed_move(move));
        assert(move.get_type() != MoveType::NULL_MOVE);

        // Castling moves are checked during generation (path safety)
        if (move.get_type() == MoveType::CASTLE_KINGSIDE ||
            move.get_type() == MoveType::CASTLE_QUEENSIDE) {
            return true;
        }

        const Color us = pos.get_side_to_move();
        const Color them = invert(us);
        const Square from = move.get_from_square();
        const Square to = move.get_to_square();

        // Calculate potential king square
        Square king_sq;
        if (get_piece_type(pos.get_piece_at(from)) == PieceType::KING) {
            king_sq = to;
        } else {
            king_sq = pos.get_bitboard(us, PieceType::KING).lsb_square();
        }
        assert(king_sq != Square::NO_SQ);

        // Calculate occupancy after move (approximate for attack checks)
        Bitboard occupancy = pos.get_occupancy();
        occupancy.clear(from);
        occupancy.set(to);

        Bitboard attackers_mask = Bitboard(~0ULL);

        if (move.get_type() == MoveType::EN_PASSANT) {
            int offset = us == Color::WHITE ? -8 : 8;
            Square cap_sq = shift(to, offset);
            occupancy.clear(cap_sq);
            attackers_mask.clear(cap_sq);
        } else if (move.get_type() == MoveType::CAPTURE ||
                   move.get_type() == MoveType::PROMOTION_CAPTURE) {
            attackers_mask.clear(to);
        }

        return !pos.is_square_attacked(
            king_sq,
            them,
            occupancy,
            attackers_mask
        );
    }

private:
    template<MoveGenType Type>
    static void generate_pawn_moves(
        const Position& pos,
        Color us,
        MoveList& moves
    ) noexcept {
        const Bitboard pawns = pos.get_bitboard(us, PieceType::PAWN);
        const Bitboard promotion_ranks = Bitboard::from_rank(Rank::RANK_1) |
                                         Bitboard::from_rank(Rank::RANK_8);

        if constexpr (Type != MoveGenType::CAPTURES) {
            const Bitboard occupancy = pos.get_occupancy();
            const std::int8_t UP = us == Color::WHITE ? 8 : -8;

            // Single Push
            Bitboard single_pushes =
                us == Color::WHITE ? (pawns << 8) : (pawns >> 8);
            single_pushes &= ~occupancy;

            Bitboard non_promotions = single_pushes & ~promotion_ranks;
            Bitboard promotions = single_pushes & promotion_ranks;

            for (Square to : non_promotions) {
                Square from = shift(to, -UP);
                moves.push_back(Move::quiet(from, to));
            }

            for (Square to : promotions) {
                Square from = shift(to, -UP);
                add_promotion_moves(from, to, us, moves);
            }

            // Double Push
            Bitboard double_pushes = us == Color::WHITE ? (single_pushes << 8)
                                                        : (single_pushes >> 8);
            double_pushes &= ~occupancy;
            double_pushes &= Bitboard::from_rank(
                us == Color::WHITE ? Rank::RANK_4 : Rank::RANK_5
            );

            for (Square to : double_pushes) {
                Square from = shift(to, -2 * UP);
                moves.push_back(Move::double_pawn_push(from, to));
            }
        }

        // Captures
        if constexpr (Type != MoveGenType::QUIETS) {
            const Bitboard them = pos.get_occupancy(invert(us));
            const int left_offset = us == Color::WHITE ? -7 : 9;
            const int right_offset = us == Color::WHITE ? -9 : 7;

            Bitboard left_captures;
            Bitboard right_captures;

            if (us == Color::WHITE) {
                left_captures = (pawns & ~Bitboard::from_file(File::FILE_A))
                                << 7;
                right_captures = (pawns & ~Bitboard::from_file(File::FILE_H))
                                 << 9;
            } else {
                left_captures =
                    (pawns & ~Bitboard::from_file(File::FILE_A)) >> 9;  // SW
                right_captures =
                    (pawns & ~Bitboard::from_file(File::FILE_H)) >> 7;  // SE
            }

            // Normal Captures
            Bitboard valid_left = left_captures & them;
            Bitboard valid_right = right_captures & them;

            // Process Left
            Bitboard left_non_promotions = valid_left & ~promotion_ranks;
            Bitboard left_promotions = valid_left & promotion_ranks;

            for (Square to : left_non_promotions) {
                Square from = shift(to, left_offset);
                moves.push_back(Move::capture(from, to, pos.get_piece_at(to)));
            }

            for (Square to : left_promotions) {
                Square from = shift(to, left_offset);
                add_promotion_moves(from, to, us, moves, pos.get_piece_at(to));
            }

            // Process Right
            Bitboard right_non_promotions = valid_right & ~promotion_ranks;
            Bitboard right_promotions = valid_right & promotion_ranks;

            for (Square to : right_non_promotions) {
                Square from = shift(to, right_offset);
                moves.push_back(Move::capture(from, to, pos.get_piece_at(to)));
            }

            for (Square to : right_promotions) {
                Square from = shift(to, right_offset);
                add_promotion_moves(from, to, us, moves, pos.get_piece_at(to));
            }

            // En Passant
            Square ep_sq = pos.get_en_passant_square();
            if (ep_sq != Square::NO_SQ) {
                Bitboard ep_bb = Bitboard::from_square(ep_sq);
                if (left_captures & ep_bb) {
                    Square from = shift(ep_sq, left_offset);
                    moves.push_back(Move::en_passant(from, ep_sq));
                }
                if (right_captures & ep_bb) {
                    Square from = shift(ep_sq, right_offset);
                    moves.push_back(Move::en_passant(from, ep_sq));
                }
            }
        }
    }

    static void add_promotion_moves(
        Square from,
        Square to,
        Color us,
        MoveList& moves,
        Piece captured_piece = Piece::NONE
    ) {
        Piece q = get_piece_from_color_type(us, PieceType::QUEEN);
        Piece r = get_piece_from_color_type(us, PieceType::ROOK);
        Piece b = get_piece_from_color_type(us, PieceType::BISHOP);
        Piece n = get_piece_from_color_type(us, PieceType::KNIGHT);

        if (captured_piece != Piece::NONE) {
            moves.push_back(
                Move::promotion_capture(from, to, captured_piece, q)
            );
            moves.push_back(
                Move::promotion_capture(from, to, captured_piece, r)
            );
            moves.push_back(
                Move::promotion_capture(from, to, captured_piece, b)
            );
            moves.push_back(
                Move::promotion_capture(from, to, captured_piece, n)
            );
        } else {
            moves.push_back(Move::promotion(from, to, q));
            moves.push_back(Move::promotion(from, to, r));
            moves.push_back(Move::promotion(from, to, b));
            moves.push_back(Move::promotion(from, to, n));
        }
    }

    template<MoveGenType Type, PieceType PType>
    static void generate_moves_for_piece(
        const Position& pos,
        Color us,
        MoveList& moves
    ) noexcept {
        const Bitboard pieces = pos.get_bitboard(us, PType);
        const Bitboard occupancy = pos.get_occupancy();

        Bitboard valid_targets;
        if constexpr (Type == MoveGenType::CAPTURES) {
            valid_targets = pos.get_occupancy(invert(us));
        } else if constexpr (Type == MoveGenType::QUIETS) {
            valid_targets = ~occupancy;
        } else {
            valid_targets = ~pos.get_occupancy(us);
        }

        for (Square from : pieces) {
            Bitboard attacks;
            if constexpr (PType == PieceType::KNIGHT) {
                attacks = Attacks::get_knight_attacks(from);
            } else if constexpr (PType == PieceType::BISHOP) {
                attacks = Attacks::get_bishop_attacks(from, occupancy);
            } else if constexpr (PType == PieceType::ROOK) {
                attacks = Attacks::get_rook_attacks(from, occupancy);
            } else if constexpr (PType == PieceType::QUEEN) {
                attacks = Attacks::get_queen_attacks(from, occupancy);
            }

            attacks &= valid_targets;

            for (Square to : attacks) {
                Piece captured = pos.get_piece_at(to);
                if (captured != Piece::NONE) {
                    moves.push_back(Move::capture(from, to, captured));
                } else {
                    moves.push_back(Move::quiet(from, to));
                }
            }
        }
    }

    template<MoveGenType Type>
    static void generate_king_moves(
        const Position& pos,
        Color us,
        MoveList& moves
    ) noexcept {
        const Bitboard king = pos.get_bitboard(us, PieceType::KING);
        assert(king.pop_count() == 1);

        const Square from = king.lsb_square();

        Bitboard valid_targets;
        if constexpr (Type == MoveGenType::CAPTURES) {
            valid_targets = pos.get_occupancy(invert(us));
        } else if constexpr (Type == MoveGenType::QUIETS) {
            valid_targets = ~pos.get_occupancy();
        } else {
            valid_targets = ~pos.get_occupancy(us);
        }

        Bitboard attacks = Attacks::get_king_attacks(from) & valid_targets;

        for (Square to : attacks) {
            Piece captured = pos.get_piece_at(to);
            if (captured != Piece::NONE) {
                moves.push_back(Move::capture(from, to, captured));
            } else {
                moves.push_back(Move::quiet(from, to));
            }
        }
    }

    static void generate_castling_moves(
        const Position& pos,
        Color us,
        MoveList& moves
    ) noexcept {
        Square king_sq = us == Color::WHITE ? Square::E1 : Square::E8;
        // Verify king is at the right square (e.g. Chess960 might be different but we assume standard based on castling logic in Position)
        if (pos.get_piece_at(king_sq) !=
            get_piece_from_color_type(us, PieceType::KING))
            return;

        // Cannot castle out of check
        if (pos.is_square_attacked(king_sq, invert(us)))
            return;

        // Kingside
        if (pos.has_kingside_castling_rights(us)) {
            Square f_sq = us == Color::WHITE ? Square::F1 : Square::F8;
            Square g_sq = us == Color::WHITE ? Square::G1 : Square::G8;

            // Path must be empty
            if (pos.get_piece_at(f_sq) == Piece::NONE &&
                pos.get_piece_at(g_sq) == Piece::NONE &&
                // Path must not be attacked
                !pos.is_square_attacked(f_sq, invert(us)) &&
                !pos.is_square_attacked(g_sq, invert(us))) {
                moves.push_back(Move::castle_kingside(king_sq, g_sq));
            }
        }

        // Queenside
        if (pos.has_queenside_castling_rights(us)) {
            Square d_sq = us == Color::WHITE ? Square::D1 : Square::D8;
            Square c_sq = us == Color::WHITE ? Square::C1 : Square::C8;
            Square b_sq = us == Color::WHITE ? Square::B1 : Square::B8;

            // Path must be empty (B, C, D)
            if (pos.get_piece_at(d_sq) == Piece::NONE &&
                pos.get_piece_at(c_sq) == Piece::NONE &&
                pos.get_piece_at(b_sq) == Piece::NONE &&
                // Path must not be attacked (D, C) - B doesn't matter for check
                !pos.is_square_attacked(d_sq, invert(us)) &&
                !pos.is_square_attacked(c_sq, invert(us))) {
                moves.push_back(Move::castle_queenside(king_sq, c_sq));
            }
        }
    }
};

}  // namespace wuttang::chess
