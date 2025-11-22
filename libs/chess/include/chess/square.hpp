#pragma once

#include <concepts>
#include <cstdint>
#include <iterator>
#include <optional>

namespace chess {

// clang-format off

enum class File : std::uint8_t {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
};

enum class Rank : std::uint8_t {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,
};

enum class Square : std::uint8_t {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
};

// clang-format on

template<typename T>
concept FileOrRank = std::same_as<T, File> || std::same_as<T, Rank>;

template<FileOrRank T>
inline constexpr std::optional<T> shift(T value, std::int8_t delta) noexcept {
    constexpr std::int8_t min = 0, max = 7;
    const std::int8_t new_val = static_cast<std::int8_t>(value) + delta;

    if (new_val < min || new_val > max)
        return std::nullopt;

    return static_cast<T>(new_val);
}

template<FileOrRank T>
inline constexpr std::optional<T> shift(
    std::optional<T> opt,
    std::int8_t delta
) noexcept {
    return opt.has_value() ? shift(*opt, delta) : std::nullopt;
}

inline constexpr Square square_from_file_rank(
    File square_file,
    Rank square_rank
) noexcept {
    return static_cast<Square>(
        static_cast<std::uint8_t>(square_rank) * 8 +
        static_cast<std::uint8_t>(square_file)
    );
}

inline constexpr File get_square_file(Square square) noexcept {
    return static_cast<File>(static_cast<std::uint8_t>(square) % 8);
}

inline constexpr Rank get_square_rank(Square square) noexcept {
    return static_cast<Rank>(static_cast<std::uint8_t>(square) / 8);
}

namespace detail {
    template<typename EnumType, EnumType beginVal, EnumType endVal>
    class EnumRange {
    public:
        using Underlying = std::underlying_type_t<EnumType>;

        class iterator {
            Underlying value_;

        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = EnumType;
            using difference_type = std::ptrdiff_t;

            explicit iterator(Underlying start): value_(start) {}
            iterator& operator++() {
                ++value_;
                return *this;
            }
            bool operator!=(const iterator& other) const {
                return value_ != other.value_;
            }
            EnumType operator*() const {
                return static_cast<EnumType>(value_);
            }
        };

        EnumRange():
                begin_(static_cast<Underlying>(beginVal)),
                end_(static_cast<Underlying>(endVal)) {}
        iterator begin() const {
            return iterator(begin_);
        }
        iterator end() const {
            return iterator(end_ + 1);
        }

    private:
        Underlying begin_, end_;
    };
}  // namespace detail

using FileRange = detail::EnumRange<File, File::FILE_A, File::FILE_H>;
using RankRange = detail::EnumRange<Rank, Rank::RANK_1, Rank::RANK_8>;
using SquareRange = detail::EnumRange<Square, Square::A1, Square::H8>;

}  // namespace chess
