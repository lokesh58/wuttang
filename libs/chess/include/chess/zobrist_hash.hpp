#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "chess/castling_rights.hpp"
#include "chess/piece.hpp"
#include "chess/square.hpp"

namespace chess {

class ZobristHash {
public:
    ZobristHash() = delete;

    static std::uint64_t get_piece_square_key(Piece piece, Square square) {
        const auto piece_idx = get_piece_index(piece);
        const auto square_idx = static_cast<std::size_t>(square);
        assert(piece_idx < 12);
        assert(square_idx < 64);
        return KEYS.piece_square_keys[piece_idx][square_idx];
    }

    static std::uint64_t get_side_to_move_key() {
        return KEYS.side_to_move_key;
    }

    static std::uint64_t get_castling_rights_key(
        CastlingRights castling_right
    ) {
        const auto rights_idx = static_cast<std::size_t>(castling_right);
        assert(rights_idx < 16);
        return KEYS.castling_right_keys[rights_idx];
    }

    static std::uint64_t get_en_passant_key(Square en_passant_square) {
        const auto ep_file = get_square_file(en_passant_square);
        const auto file_idx = static_cast<std::size_t>(ep_file);
        assert(file_idx < 8);
        return KEYS.en_passant_file_keys[file_idx];
    }

private:
    struct Keys {
        std::array<std::array<std::uint64_t, 64>, 12> piece_square_keys;
        std::uint64_t side_to_move_key;
        std::array<std::uint64_t, 16> castling_right_keys;
        std::array<std::uint64_t, 8> en_passant_file_keys;
    };

    static constexpr Keys KEYS = [] {
        Keys keys{};

        std::uint64_t state = 1070372;
        auto next_rng = [&] {
            std::uint64_t z = (state += 0x9e3779b97f4a7c15ULL);
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
            z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
            return z ^ (z >> 31);
        };

        for (auto& piece_keys : keys.piece_square_keys) {
            for (auto& key : piece_keys) {
                key = next_rng();
            }
        }
        keys.side_to_move_key = next_rng();
        for (auto& key : keys.castling_right_keys) {
            key = next_rng();
        }
        for (auto& key : keys.en_passant_file_keys) {
            key = next_rng();
        }
        return keys;
    }();
};

}  // namespace chess
