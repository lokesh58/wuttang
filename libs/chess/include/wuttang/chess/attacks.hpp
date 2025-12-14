#pragma once

#include <array>
#include <cstddef>
#include <wuttang/chess/bitboard.hpp>
#include <wuttang/chess/color.hpp>
#include <wuttang/chess/square.hpp>

namespace wuttang::chess {

namespace detail {

    enum class Direction {
        NORTH,
        NORTH_EAST,
        EAST,
        SOUTH_EAST,
        SOUTH,
        SOUTH_WEST,
        WEST,
        NORTH_WEST,
        COUNT  // Number of directions
    };

    static constexpr Bitboard compute_king_attacks(Square sq) noexcept {
        Bitboard attacks;
        Bitboard b = Bitboard::from_square(sq);

        Bitboard not_h_file = ~Bitboard::from_file(File::FILE_H);
        Bitboard not_a_file = ~Bitboard::from_file(File::FILE_A);

        attacks |= ((b & not_h_file) << 9);  // NE
        attacks |= (b << 8);                 // N
        attacks |= ((b & not_a_file) << 7);  // NW
        attacks |= ((b & not_h_file) << 1);  // E
        attacks |= ((b & not_a_file) >> 1);  // W
        attacks |= ((b & not_h_file) >> 7);  // SE
        attacks |= (b >> 8);                 // S
        attacks |= ((b & not_a_file) >> 9);  // SW

        return attacks;
    }

    static constexpr Bitboard compute_knight_attacks(Square sq) noexcept {
        Bitboard attacks;
        Bitboard b = Bitboard::from_square(sq);
        Bitboard not_h_file = ~Bitboard::from_file(File::FILE_H);
        Bitboard not_gh_file =
            ~(Bitboard::from_file(File::FILE_G) |
              Bitboard::from_file(File::FILE_H));
        Bitboard not_a_file = ~Bitboard::from_file(File::FILE_A);
        Bitboard not_ab_file =
            ~(Bitboard::from_file(File::FILE_A) |
              Bitboard::from_file(File::FILE_B));

        // All 8 knight moves
        attacks |= ((b & not_h_file) << 17);   // NNE
        attacks |= ((b & not_gh_file) << 10);  // ENE
        attacks |= ((b & not_gh_file) >> 6);   // ESE
        attacks |= ((b & not_h_file) >> 15);   // SSE
        attacks |= ((b & not_a_file) >> 17);   // SSW
        attacks |= ((b & not_ab_file) >> 10);  // WSW
        attacks |= ((b & not_ab_file) << 6);   // WNW
        attacks |= ((b & not_a_file) << 15);   // NNW

        return attacks;
    }

    static constexpr Bitboard compute_pawn_attacks(
        Color color,
        Square sq
    ) noexcept {
        Bitboard attacks;
        Bitboard b = Bitboard::from_square(sq);
        Bitboard not_h_file = ~Bitboard::from_file(File::FILE_H);
        Bitboard not_a_file = ~Bitboard::from_file(File::FILE_A);

        if (color == Color::WHITE) {
            attacks |= ((b & not_h_file) << 9);  // NE capture
            attacks |= ((b & not_a_file) << 7);  // NW capture
        } else {                                 // Color::BLACK
            attacks |= ((b & not_h_file) >> 7);  // SE capture
            attacks |= ((b & not_a_file) >> 9);  // SW capture
        }
        return attacks;
    }

    // Computes a ray from 'sq' in 'dir' direction, extending to the board edge.
    static constexpr Bitboard compute_ray(Square sq, Direction dir) noexcept {
        Bitboard ray;
        int r = static_cast<int>(get_square_rank(sq));
        int f = static_cast<int>(get_square_file(sq));

        int dr = 0;  // delta row
        int df = 0;  // delta file

        switch (dir) {
            case Direction::NORTH:
                dr = 1;
                break;
            case Direction::SOUTH:
                dr = -1;
                break;
            case Direction::EAST:
                df = 1;
                break;
            case Direction::WEST:
                df = -1;
                break;
            case Direction::NORTH_EAST:
                dr = 1;
                df = 1;
                break;
            case Direction::NORTH_WEST:
                dr = 1;
                df = -1;
                break;
            case Direction::SOUTH_EAST:
                dr = -1;
                df = 1;
                break;
            case Direction::SOUTH_WEST:
                dr = -1;
                df = -1;
                break;
            case Direction::COUNT:
                break;  // Should not be reached
        }

        for (int i = 1; i < 8; ++i) {  // Max 7 squares in any direction
            int nr = r + dr * i;
            int nf = f + df * i;
            if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
                ray |= Bitboard::from_square(get_square_from_file_rank(
                    static_cast<File>(nf),
                    static_cast<Rank>(nr)
                ));
            } else {
                break;  // Off board
            }
        }
        return ray;
    }
}  // namespace detail

class Attacks {
public:
    using Direction = detail::Direction;  // Expose if needed or keep internal

    // --- Leaper Attacks ---

    static constexpr Bitboard get_king_attacks(Square sq) noexcept {
        return KING_ATTACKS[static_cast<std::size_t>(sq)];
    }

    static constexpr Bitboard get_knight_attacks(Square sq) noexcept {
        return KNIGHT_ATTACKS[static_cast<std::size_t>(sq)];
    }

    // --- Pawn Attacks ---

    static constexpr Bitboard get_pawn_attacks(Color side, Square sq) noexcept {
        if (side == Color::WHITE) {
            return WHITE_PAWN_ATTACKS[static_cast<std::size_t>(sq)];
        } else {
            return BLACK_PAWN_ATTACKS[static_cast<std::size_t>(sq)];
        }
    }

    // --- Sliding Attacks ---

    static constexpr Bitboard get_bishop_attacks(
        Square sq,
        Bitboard occupancy
    ) noexcept {
        return get_ray_attacks(sq, occupancy, Direction::NORTH_EAST) |
               get_ray_attacks(sq, occupancy, Direction::SOUTH_EAST) |
               get_ray_attacks(sq, occupancy, Direction::SOUTH_WEST) |
               get_ray_attacks(sq, occupancy, Direction::NORTH_WEST);
    }

    static constexpr Bitboard get_rook_attacks(
        Square sq,
        Bitboard occupancy
    ) noexcept {
        return get_ray_attacks(sq, occupancy, Direction::NORTH) |
               get_ray_attacks(sq, occupancy, Direction::EAST) |
               get_ray_attacks(sq, occupancy, Direction::SOUTH) |
               get_ray_attacks(sq, occupancy, Direction::WEST);
    }

    static constexpr Bitboard get_queen_attacks(
        Square sq,
        Bitboard occupancy
    ) noexcept {
        return get_bishop_attacks(sq, occupancy) |
               get_rook_attacks(sq, occupancy);
    }

private:
    // Precomputed Lookups for Leapers
    static constexpr std::array<Bitboard, 64> KING_ATTACKS = [] {
        std::array<Bitboard, 64> table{};
        for (int i = 0; i < 64; ++i)
            table[i] = detail::compute_king_attacks(static_cast<Square>(i));
        return table;
    }();

    static constexpr std::array<Bitboard, 64> KNIGHT_ATTACKS = [] {
        std::array<Bitboard, 64> table{};
        for (int i = 0; i < 64; ++i)
            table[i] = detail::compute_knight_attacks(static_cast<Square>(i));
        return table;
    }();

    static constexpr std::array<Bitboard, 64> WHITE_PAWN_ATTACKS = [] {
        std::array<Bitboard, 64> table{};
        for (int i = 0; i < 64; ++i)
            table[i] = detail::compute_pawn_attacks(
                Color::WHITE,
                static_cast<Square>(i)
            );
        return table;
    }();

    static constexpr std::array<Bitboard, 64> BLACK_PAWN_ATTACKS = [] {
        std::array<Bitboard, 64> table{};
        for (int i = 0; i < 64; ++i)
            table[i] = detail::compute_pawn_attacks(
                Color::BLACK,
                static_cast<Square>(i)
            );
        return table;
    }();

    // Precomputed all 8 rays for all 64 squares.
    static constexpr std::array<
        std::array<Bitboard, static_cast<std::size_t>(Direction::COUNT)>,
        64
    >
        RAYS = [] {
            std::array<
                std::
                    array<Bitboard, static_cast<std::size_t>(Direction::COUNT)>,
                64
            >
                table{};
            for (int i = 0; i < 64; ++i) {
                for (int d = 0; d < static_cast<int>(Direction::COUNT); ++d) {
                    table[i][d] = detail::compute_ray(
                        static_cast<Square>(i),
                        static_cast<Direction>(d)
                    );
                }
            }
            return table;
        }();

    // Calculates sliding attacks for a given square and occupancy using
    // precomputed rays.
    static constexpr Bitboard get_ray_attacks(
        Square sq,
        Bitboard occupancy,
        Direction dir
    ) noexcept {
        Bitboard ray =
            RAYS[static_cast<std::size_t>(sq)][static_cast<std::size_t>(dir)];
        Bitboard blockers = ray & occupancy;

        if (!blockers) {
            return ray;  // No blockers, full ray is attackable
        }

        Square first_blocker = Square::NO_SQ;

        // Determine the "first" blocker in the given direction.
        // For directions that increase square index (N, NE, E), LSB is the
        // closest. For directions that decrease square index (S, SW, W), MSB is
        // the closest.
        bool is_positive_dir =
            (dir == Direction::NORTH || dir == Direction::NORTH_EAST ||
             dir == Direction::EAST ||
             dir == Direction::NORTH_WEST);  // NW increases index for A-H,
                                             // decreases for rank. For bitboard
                                             // (0-63), N, NE, E, NW shifts are
                                             // positive.

        if (is_positive_dir) {
            first_blocker = blockers.lsb_square();
        } else {
            first_blocker = blockers.msb_square();
        }

        // Remove the ray segment beyond the first blocker.
        // The idea is: full_ray XOR
        // ray_from_blocker_excluding_blocker_square This effectively keeps the
        // ray from 'sq' up to and including 'first_blocker'.
        return ray ^ RAYS[static_cast<std::size_t>(first_blocker)]
                         [static_cast<std::size_t>(dir)];
    }
};

}  // namespace wuttang::chess
