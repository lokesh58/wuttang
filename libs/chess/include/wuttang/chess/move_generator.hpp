#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <wuttang/chess/attacks.hpp>
#include <wuttang/chess/bitboard.hpp>
#include <wuttang/chess/castling_rights.hpp>
#include <wuttang/chess/color.hpp>
#include <wuttang/chess/move.hpp>
#include <wuttang/chess/move_list.hpp>
#include <wuttang/chess/piece.hpp>
#include <wuttang/chess/position.hpp>
#include <wuttang/chess/square.hpp>

namespace wuttang::chess {

enum class MoveGenType {
    PSEUDO_LEGAL,  // All pseudo-legal moves
    LEGAL,         // All legal moves
    CAPTURE,       // Pseudo-legal captures
    QUIET          // Pseudo-legal quiet moves
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

        if constexpr (Type != MoveGenType::CAPTURE) {
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

        if constexpr (Type != MoveGenType::CAPTURE) {
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
        if constexpr (Type != MoveGenType::QUIET) {
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
        if constexpr (Type == MoveGenType::CAPTURE) {
            valid_targets = pos.get_occupancy(invert(us));
        } else if constexpr (Type == MoveGenType::QUIET) {
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
        if constexpr (Type == MoveGenType::CAPTURE) {
            valid_targets = pos.get_occupancy(invert(us));
        } else if constexpr (Type == MoveGenType::QUIET) {
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
        const bool has_kingside = pos.has_kingside_castling_rights(us);
        const bool has_queenside = pos.has_queenside_castling_rights(us);
        if (!has_kingside && !has_queenside) {
            return;
        }
        const Rank rank = us == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8;
        const Square king_sq =
            get_square_from_file_rank(pos.get_king_file(), rank);

        // Cannot castle out of check
        if (pos.is_square_attacked(king_sq, invert(us)))
            return;

        auto check_castling = [&](File rook_file,
                                  File target_king_file,
                                  File target_rook_file,
                                  auto move_factory) {
            const int k_f = static_cast<int>(pos.get_king_file());
            const int r_f = static_cast<int>(rook_file);
            const int tk_f = static_cast<int>(target_king_file);
            const int tr_f = static_cast<int>(target_rook_file);

            // 1. Check Path Emptiness
            // All squares between min and max of (K, R, TargetK, TargetR) must
            // be empty EXCEPT for the King's start square and Rook's start
            // square.
            const int min_f = std::min({k_f, r_f, tk_f, tr_f});
            const int max_f = std::max({k_f, r_f, tk_f, tr_f});

            for (int f = min_f; f <= max_f; ++f) {
                if (f == k_f || f == r_f)
                    continue;
                if (pos.get_piece_at(
                        get_square_from_file_rank(static_cast<File>(f), rank)
                    ) != Piece::NONE)
                    return;
            }

            // 2. Check King Safety on Path
            // King must not pass through check or end up in check.
            // Start square is already checked.
            const int dir = (tk_f > k_f) ? 1 : -1;
            if (k_f != tk_f) {
                for (int f = k_f + dir;; f += dir) {
                    Square sq =
                        get_square_from_file_rank(static_cast<File>(f), rank);
                    if (pos.is_square_attacked(sq, invert(us)))
                        return;
                    if (f == tk_f)
                        break;
                }
            }

            moves.push_back(move_factory(
                king_sq,
                get_square_from_file_rank(target_king_file, rank)
            ));
        };

        // Kingside
        if (has_kingside) {
            check_castling(
                pos.get_kingside_rook_file(),
                File::FILE_G,
                File::FILE_F,
                Move::castle_kingside
            );
        }

        // Queenside
        if (has_queenside) {
            check_castling(
                pos.get_queenside_rook_file(),
                File::FILE_C,
                File::FILE_D,
                Move::castle_queenside
            );
        }
    }
};

}  // namespace wuttang::chess
