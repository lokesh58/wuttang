#include <gtest/gtest.h>

#include "chess/bitboard.hpp"
#include "chess/square.hpp"

namespace chess {

TEST(BitboardTest, DefaultConstructor) {
    constexpr Bitboard bb;
    EXPECT_EQ(bb.value(), 0ULL);
    EXPECT_EQ(bb.pop_count(), 0);
    EXPECT_FALSE(static_cast<bool>(bb));
}

TEST(BitboardTest, ValueConstructor) {
    constexpr Bitboard bb(0x1234567890ABCDEF);
    EXPECT_EQ(bb.value(), 0x1234567890ABCDEF);
}

TEST(BitboardTest, FromSquare) {
    constexpr Bitboard bb = Bitboard::from_square(Square::E4);
    EXPECT_EQ(bb.pop_count(), 1);
    EXPECT_TRUE(bb.get(Square::E4));
    EXPECT_FALSE(bb.get(Square::E5));
}

TEST(BitboardTest, FromFile) {
    constexpr Bitboard bb = Bitboard::from_file(File::FILE_D);
    EXPECT_EQ(bb.pop_count(), 8);
    for (auto sq : bb) {
        EXPECT_EQ(get_square_file(sq), File::FILE_D);
    }
}

TEST(BitboardTest, FromRank) {
    constexpr Bitboard bb = Bitboard::from_rank(Rank::RANK_3);
    EXPECT_EQ(bb.pop_count(), 8);
    for (auto sq : bb) {
        EXPECT_EQ(get_square_rank(sq), Rank::RANK_3);
    }
}

TEST(BitboardTest, SetGetClearToggle) {
    Bitboard bb;

    // Set
    bb.set(Square::A1);
    EXPECT_TRUE(bb.get(Square::A1));
    EXPECT_EQ(bb.pop_count(), 1);

    bb.set(Square::H8);
    EXPECT_TRUE(bb.get(Square::H8));
    EXPECT_EQ(bb.pop_count(), 2);

    // Toggle
    bb.toggle(Square::A1);  // off
    EXPECT_FALSE(bb.get(Square::A1));
    EXPECT_EQ(bb.pop_count(), 1);

    bb.toggle(Square::A1);  // on
    EXPECT_TRUE(bb.get(Square::A1));
    EXPECT_EQ(bb.pop_count(), 2);

    // Clear
    bb.clear(Square::H8);
    EXPECT_FALSE(bb.get(Square::H8));
    EXPECT_EQ(bb.pop_count(), 1);
}

TEST(BitboardTest, PopCount) {
    Bitboard bb(0ULL);
    EXPECT_EQ(bb.pop_count(), 0);

    bb.set(Square::A1);
    EXPECT_EQ(bb.pop_count(), 1);

    bb.set(Square::H8);
    EXPECT_EQ(bb.pop_count(), 2);

    bb = Bitboard(~0ULL);
    EXPECT_EQ(bb.pop_count(), 64);
}

TEST(BitboardTest, LsbOperations) {
    Bitboard bb;
    bb.set(Square::E4);
    bb.set(Square::A1);  // LSB
    bb.set(Square::H8);

    EXPECT_EQ(bb.lsb_square(), Square::A1);
    EXPECT_EQ(bb.lsb_index(), static_cast<int>(Square::A1));

    Square sq = bb.pop_lsb();
    EXPECT_EQ(sq, Square::A1);
    EXPECT_EQ(bb.lsb_square(), Square::E4);

    sq = bb.pop_lsb();
    EXPECT_EQ(sq, Square::E4);
    EXPECT_EQ(bb.lsb_square(), Square::H8);

    sq = bb.pop_lsb();
    EXPECT_EQ(sq, Square::H8);

    // Empty
    sq = bb.pop_lsb();
    EXPECT_EQ(sq, Square::NO_SQ);
}

TEST(BitboardTest, MsbOperations) {
    Bitboard bb;
    bb.set(Square::A1);
    bb.set(Square::E4);  // Middle
    bb.set(Square::H8);  // MSB (Square 63)

    // msb_index for H8 is 63.
    EXPECT_EQ(bb.msb_square(), Square::H8);
    EXPECT_EQ(bb.msb_index(), 63);

    // Remove H8
    bb.clear(Square::H8);
    // Now MSB is E4 (Square 28).
    EXPECT_EQ(bb.msb_square(), Square::E4);
    EXPECT_EQ(bb.msb_index(), 28);

    // Remove E4
    bb.clear(Square::E4);
    // Now MSB is A1 (Square 0).
    EXPECT_EQ(bb.msb_square(), Square::A1);
    EXPECT_EQ(bb.msb_index(), 0);

    // Remove A1
    bb.clear(Square::A1);
    // Now MSB is NO_SQ
    EXPECT_EQ(bb.msb_square(), Square::NO_SQ);
    EXPECT_EQ(bb.msb_index(), -1);
}

TEST(BitboardTest, MoreThanOne) {
    Bitboard bb;
    EXPECT_FALSE(bb.more_than_one());

    bb.set(Square::A1);
    EXPECT_FALSE(bb.more_than_one());

    bb.set(Square::B1);
    EXPECT_TRUE(bb.more_than_one());
}

TEST(BitboardTest, Operators) {
    Bitboard b1(0x0F0F0F0F0F0F0F0FULL);
    Bitboard b2(0xF0F0F0F0F0F0F0F0ULL);

    // ~
    EXPECT_EQ((~b1).value(), 0xF0F0F0F0F0F0F0F0ULL);

    // &
    EXPECT_EQ((b1 & b2).value(), 0ULL);

    // |
    EXPECT_EQ((b1 | b2).value(), ~0ULL);

    // ^
    EXPECT_EQ((b1 ^ b2).value(), ~0ULL);

    // <<
    Bitboard b3(1ULL);
    EXPECT_EQ((b3 << 1).value(), 2ULL);

    // >>
    Bitboard b4(2ULL);
    EXPECT_EQ((b4 >> 1).value(), 1ULL);
}

TEST(BitboardTest, Iterator) {
    Bitboard bb;
    bb.set(Square::A1);
    bb.set(Square::E4);
    bb.set(Square::H8);

    int count = 0;
    std::vector<Square> squares;
    for (Square sq : bb) {
        squares.push_back(sq);
        count++;
    }

    EXPECT_EQ(count, 3);
    EXPECT_EQ(squares[0], Square::A1);
    EXPECT_EQ(squares[1], Square::E4);
    EXPECT_EQ(squares[2], Square::H8);
}

TEST(BitboardTest, ConstIterator) {
    // Ensure it works on const objects
    const Bitboard bb =
        Bitboard::from_square(Square::E4) | Bitboard::from_square(Square::H8);

    int count = 0;
    for (auto it = bb.begin(); it != bb.end(); ++it) {
        count++;
    }
    EXPECT_EQ(count, 2);
}

}  // namespace chess
