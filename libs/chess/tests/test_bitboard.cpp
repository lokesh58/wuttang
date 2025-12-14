#include <gtest/gtest.h>

#include "chess/bitboard.hpp"
#include "chess/square.hpp"

TEST(BitboardTest, DefaultConstructor) {
    constexpr chess::Bitboard bb;
    EXPECT_EQ(bb.value(), 0ULL);
    EXPECT_EQ(bb.pop_count(), 0);
    EXPECT_FALSE(static_cast<bool>(bb));
}

TEST(BitboardTest, ValueConstructor) {
    constexpr chess::Bitboard bb(0x1234567890ABCDEF);
    EXPECT_EQ(bb.value(), 0x1234567890ABCDEF);
}

TEST(BitboardTest, FromSquare) {
    constexpr chess::Bitboard bb =
        chess::Bitboard::from_square(chess::Square::E4);
    EXPECT_EQ(bb.pop_count(), 1);
    EXPECT_TRUE(bb.get(chess::Square::E4));
    EXPECT_FALSE(bb.get(chess::Square::E5));
}

TEST(BitboardTest, FromFile) {
    constexpr chess::Bitboard bb =
        chess::Bitboard::from_file(chess::File::FILE_D);
    EXPECT_EQ(bb.pop_count(), 8);
    for (auto sq : bb) {
        EXPECT_EQ(chess::get_square_file(sq), chess::File::FILE_D);
    }
}

TEST(BitboardTest, FromRank) {
    constexpr chess::Bitboard bb =
        chess::Bitboard::from_rank(chess::Rank::RANK_3);
    EXPECT_EQ(bb.pop_count(), 8);
    for (auto sq : bb) {
        EXPECT_EQ(chess::get_square_rank(sq), chess::Rank::RANK_3);
    }
}

TEST(BitboardTest, SetGetClearToggle) {
    chess::Bitboard bb;

    // Set
    bb.set(chess::Square::A1);
    EXPECT_TRUE(bb.get(chess::Square::A1));
    EXPECT_EQ(bb.pop_count(), 1);

    bb.set(chess::Square::H8);
    EXPECT_TRUE(bb.get(chess::Square::H8));
    EXPECT_EQ(bb.pop_count(), 2);

    // Toggle
    bb.toggle(chess::Square::A1);  // off
    EXPECT_FALSE(bb.get(chess::Square::A1));
    EXPECT_EQ(bb.pop_count(), 1);

    bb.toggle(chess::Square::A1);  // on
    EXPECT_TRUE(bb.get(chess::Square::A1));
    EXPECT_EQ(bb.pop_count(), 2);

    // Clear
    bb.clear(chess::Square::H8);
    EXPECT_FALSE(bb.get(chess::Square::H8));
    EXPECT_EQ(bb.pop_count(), 1);
}

TEST(BitboardTest, PopCount) {
    chess::Bitboard bb(0ULL);
    EXPECT_EQ(bb.pop_count(), 0);

    bb.set(chess::Square::A1);
    EXPECT_EQ(bb.pop_count(), 1);

    bb.set(chess::Square::H8);
    EXPECT_EQ(bb.pop_count(), 2);

    bb = chess::Bitboard(~0ULL);
    EXPECT_EQ(bb.pop_count(), 64);
}

TEST(BitboardTest, LsbOperations) {
    chess::Bitboard bb;
    bb.set(chess::Square::E4);
    bb.set(chess::Square::A1);  // LSB
    bb.set(chess::Square::H8);

    EXPECT_EQ(bb.lsb_square(), chess::Square::A1);
    EXPECT_EQ(bb.lsb_index(), static_cast<int>(chess::Square::A1));

    chess::Square sq = bb.pop_lsb();
    EXPECT_EQ(sq, chess::Square::A1);
    EXPECT_EQ(bb.lsb_square(), chess::Square::E4);

    sq = bb.pop_lsb();
    EXPECT_EQ(sq, chess::Square::E4);
    EXPECT_EQ(bb.lsb_square(), chess::Square::H8);

    sq = bb.pop_lsb();
    EXPECT_EQ(sq, chess::Square::H8);

    // Empty
    sq = bb.pop_lsb();
    EXPECT_EQ(sq, chess::Square::NO_SQ);
}

TEST(BitboardTest, MsbOperations) {
    chess::Bitboard bb;
    bb.set(chess::Square::A1);
    bb.set(chess::Square::E4);  // Middle
    bb.set(chess::Square::H8);  // MSB (Square 63)

    // msb_index for H8 is 63.
    EXPECT_EQ(bb.msb_square(), chess::Square::H8);
    EXPECT_EQ(bb.msb_index(), 63);

    // Remove H8
    bb.clear(chess::Square::H8);
    // Now MSB is E4 (Square 28).
    EXPECT_EQ(bb.msb_square(), chess::Square::E4);
    EXPECT_EQ(bb.msb_index(), 28);

    // Remove E4
    bb.clear(chess::Square::E4);
    // Now MSB is A1 (Square 0).
    EXPECT_EQ(bb.msb_square(), chess::Square::A1);
    EXPECT_EQ(bb.msb_index(), 0);

    // Remove A1
    bb.clear(chess::Square::A1);
    // Now MSB is NO_SQ
    EXPECT_EQ(bb.msb_square(), chess::Square::NO_SQ);
    EXPECT_EQ(bb.msb_index(), -1);
}

TEST(BitboardTest, MoreThanOne) {
    chess::Bitboard bb;
    EXPECT_FALSE(bb.more_than_one());

    bb.set(chess::Square::A1);
    EXPECT_FALSE(bb.more_than_one());

    bb.set(chess::Square::B1);
    EXPECT_TRUE(bb.more_than_one());
}

TEST(BitboardTest, Operators) {
    chess::Bitboard b1(0x0F0F0F0F0F0F0F0FULL);
    chess::Bitboard b2(0xF0F0F0F0F0F0F0F0ULL);

    // ~
    EXPECT_EQ((~b1).value(), 0xF0F0F0F0F0F0F0F0ULL);

    // &
    EXPECT_EQ((b1 & b2).value(), 0ULL);

    // |
    EXPECT_EQ((b1 | b2).value(), ~0ULL);

    // ^
    EXPECT_EQ((b1 ^ b2).value(), ~0ULL);

    // <<
    chess::Bitboard b3(1ULL);
    EXPECT_EQ((b3 << 1).value(), 2ULL);

    // >>
    chess::Bitboard b4(2ULL);
    EXPECT_EQ((b4 >> 1).value(), 1ULL);
}

TEST(BitboardTest, Iterator) {
    chess::Bitboard bb;
    bb.set(chess::Square::A1);
    bb.set(chess::Square::E4);
    bb.set(chess::Square::H8);

    int count = 0;
    std::vector<chess::Square> squares;
    for (chess::Square sq : bb) {
        squares.push_back(sq);
        count++;
    }

    EXPECT_EQ(count, 3);
    EXPECT_EQ(squares[0], chess::Square::A1);
    EXPECT_EQ(squares[1], chess::Square::E4);
    EXPECT_EQ(squares[2], chess::Square::H8);
}

TEST(BitboardTest, ConstIterator) {
    // Ensure it works on const objects
    const chess::Bitboard bb = chess::Bitboard::from_square(chess::Square::E4) |
                               chess::Bitboard::from_square(chess::Square::H8);

    int count = 0;
    for (auto it = bb.begin(); it != bb.end(); ++it) {
        count++;
    }
    EXPECT_EQ(count, 2);
}
