#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <wuttang/chess/castling_rights.hpp>
#include <wuttang/chess/piece.hpp>
#include <wuttang/chess/square.hpp>

namespace wuttang::chess {

class ZobristHash {
public:
    ZobristHash() = delete;

    static constexpr std::uint64_t get_piece_square_key(
        Piece piece,
        Square square
    ) {
        const auto piece_idx = static_cast<std::size_t>(piece);
        const auto square_idx = static_cast<std::size_t>(square);
        assert(piece_idx < 15);
        assert(square_idx < 64);
        return KEYS.piece_square_keys[piece_idx][square_idx];
    }

    static constexpr std::uint64_t get_side_to_move_key() noexcept {
        return KEYS.side_to_move_key;
    }

    static constexpr std::uint64_t get_castling_rights_key(
        CastlingRights castling_rights
    ) noexcept {
        const auto rights_index = static_cast<std::size_t>(castling_rights);
        assert(rights_index < 16);
        return KEYS.castling_right_keys[rights_index];
    }

    static constexpr std::uint64_t get_en_passant_key(
        Square en_passant_sq
    ) noexcept {
        const auto ep_file = get_square_file(en_passant_sq);
        const auto ep_file_index = static_cast<std::size_t>(ep_file);
        assert(ep_file_index < 8);
        return KEYS.en_passant_file_keys[ep_file_index];
    }

private:
    struct Keys {
        std::array<std::array<std::uint64_t, 64>, 15> piece_square_keys;
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

}  // namespace wuttang::chess
