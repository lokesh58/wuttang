#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <iterator>

#include "chess/square.hpp"

namespace chess {

class Bitboard {
public:
    constexpr Bitboard() noexcept : bits_(0) {}
    constexpr explicit Bitboard(std::uint64_t bits) noexcept : bits_(bits) {}

    static constexpr Bitboard from_square(Square sq) noexcept {
        return Bitboard(1ULL << static_cast<std::uint8_t>(sq));
    }

    static constexpr Bitboard from_file(File f) noexcept {
        return Bitboard(0x0101010101010101ULL << static_cast<std::uint8_t>(f));
    }

    // --- Bit Manipulation ---
    constexpr bool get(Square sq) const noexcept {
        return (bits_ & (1ULL << static_cast<std::uint8_t>(sq))) != 0;
    }
    constexpr Bitboard& set(Square sq) noexcept {
        bits_ |= (1ULL << static_cast<std::uint8_t>(sq));
        return *this;
    }
    constexpr Bitboard& clear(Square sq) noexcept {
        bits_ &= ~(1ULL << static_cast<std::uint8_t>(sq));
        return *this;
    }
    constexpr Bitboard& toggle(Square sq) noexcept {
        bits_ ^= (1ULL << static_cast<std::uint8_t>(sq));
        return *this;
    }
    constexpr int pop_count() const noexcept {
        return std::popcount(bits_);
    }
    constexpr Square pop_lsb() noexcept {
        if (bits_ == 0ULL)
            return Square::NO_SQ;  // Sentinel for empty
        int index = lsb_index();
        bits_ &= bits_ - 1;  // Clear LSB
        return static_cast<Square>(index);
    }
    constexpr int lsb_index() const noexcept {
        return std::countr_zero(bits_);
    }
    constexpr Square lsb_square() const noexcept {
        return static_cast<Square>(lsb_index());
    }
    constexpr int msb_index() const noexcept {
        return 63 - std::countl_zero(bits_);
    }
    constexpr Square msb_square() const noexcept {
        if (bits_ == 0ULL)
            return Square::NO_SQ;
        return static_cast<Square>(msb_index());
    }
    constexpr std::uint64_t value() const noexcept {
        return bits_;
    }
    constexpr bool more_than_one() const noexcept {
        return (bits_ & (bits_ - 1)) != 0ULL;
    }

    // --- Operators ---
    constexpr Bitboard operator~() const noexcept {
        return Bitboard(~bits_);
    }
    constexpr Bitboard operator&(const Bitboard& other) const noexcept {
        return Bitboard(bits_ & other.bits_);
    }
    constexpr Bitboard& operator&=(const Bitboard& other) noexcept {
        bits_ &= other.bits_;
        return *this;
    }
    constexpr Bitboard operator|(const Bitboard& other) const noexcept {
        return Bitboard(bits_ | other.bits_);
    }
    constexpr Bitboard& operator|=(const Bitboard& other) noexcept {
        bits_ |= other.bits_;
        return *this;
    }
    constexpr Bitboard operator^(const Bitboard& other) const noexcept {
        return Bitboard(bits_ ^ other.bits_);
    }
    constexpr Bitboard& operator^=(const Bitboard& other) noexcept {
        bits_ ^= other.bits_;
        return *this;
    }
    constexpr Bitboard operator<<(int shift) const noexcept {
        return Bitboard(bits_ << shift);
    }
    constexpr Bitboard& operator<<=(int shift) noexcept {
        bits_ <<= shift;
        return *this;
    }
    constexpr Bitboard operator>>(int shift) const noexcept {
        return Bitboard(bits_ >> shift);
    }
    constexpr Bitboard& operator>>=(int shift) noexcept {
        bits_ >>= shift;
        return *this;
    }
    constexpr bool operator==(const Bitboard& other) const noexcept {
        return bits_ == other.bits_;
    }
    constexpr bool operator!=(const Bitboard& other) const noexcept {
        return bits_ != other.bits_;
    }
    constexpr explicit operator bool() const noexcept {
        return bits_ != 0;
    }

    // --- Iterator support ---
    class Iterator;

    constexpr Iterator begin() const noexcept;
    constexpr Iterator end() const noexcept;

private:
    std::uint64_t bits_;
};

class Bitboard::Iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Square;
    using difference_type = std::ptrdiff_t;
    using pointer = Square*;
    using reference = Square&;

    // Default constructor creates an end iterator
    constexpr Iterator() noexcept : bb_(), current_sq_(Square::NO_SQ) {}
    constexpr explicit Iterator(const Bitboard& bb) noexcept :
            bb_(bb),
            current_sq_(bb.lsb_square()) {}

    constexpr Square operator*() const noexcept {
        return current_sq_;
    }
    constexpr Iterator& operator++() noexcept {
        bb_.pop_lsb();
        current_sq_ = bb_.lsb_square();
        return *this;
    }
    constexpr Iterator operator++(int) noexcept {
        Iterator temp = *this;
        ++(*this);
        return temp;
    }
    constexpr bool operator==(const Iterator& other) const noexcept {
        return current_sq_ == other.current_sq_;
    }
    constexpr bool operator!=(const Iterator& other) const noexcept {
        return current_sq_ != other.current_sq_;
    }

private:
    Bitboard bb_;
    Square current_sq_;
};

constexpr Bitboard::Iterator Bitboard::begin() const noexcept {
    return Iterator(*this);
}
constexpr Bitboard::Iterator Bitboard::end() const noexcept {
    return Iterator();
}

}  // namespace chess
