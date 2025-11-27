#include <gtest/gtest.h>
#include "chess/square.hpp"
#include <vector>
#include <algorithm>
#include <ranges>

using namespace chess;

TEST(IteratorTest, FileRangeForward) {
    std::vector<File> files;
    for (auto f : FileRange()) {
        files.push_back(f);
    }
    ASSERT_EQ(files.size(), 8);
    EXPECT_EQ(files[0], File::FILE_A);
    EXPECT_EQ(files[7], File::FILE_H);
}

TEST(IteratorTest, FileRangeReverse) {
    std::vector<File> files;
    for (auto it = FileRange().rbegin(); it != FileRange().rend(); ++it) {
        files.push_back(*it);
    }
    ASSERT_EQ(files.size(), 8);
    EXPECT_EQ(files[0], File::FILE_H);
    EXPECT_EQ(files[7], File::FILE_A);
}

TEST(IteratorTest, RankRangeReverse) {
    std::vector<Rank> ranks;
    for (auto it = RankRange().rbegin(); it != RankRange().rend(); ++it) {
        ranks.push_back(*it);
    }
    ASSERT_EQ(ranks.size(), 8);
    EXPECT_EQ(ranks[0], Rank::RANK_8);
    EXPECT_EQ(ranks[7], Rank::RANK_1);
}

TEST(IteratorTest, SquareRangeReverse) {
    std::vector<Square> squares;
    for (auto it = SquareRange().rbegin(); it != SquareRange().rend(); ++it) {
        squares.push_back(*it);
    }
    ASSERT_EQ(squares.size(), 64);
    EXPECT_EQ(squares[0], Square::H8);
    EXPECT_EQ(squares[63], Square::A1);
}

TEST(IteratorTest, StdReverseIterator) {
    SquareRange range;
    auto it = std::find(range.rbegin(), range.rend(), Square::E4);
    EXPECT_NE(it, range.rend());
    EXPECT_EQ(*it, Square::E4);
}

TEST(IteratorTest, RangesReverse) {
    std::vector<Rank> ranks;
    auto range = RankRange{};
    for (auto rank : std::views::reverse(range)) {
        ranks.push_back(rank);
    }
    ASSERT_EQ(ranks.size(), 8);
    EXPECT_EQ(ranks[0], Rank::RANK_8);
    EXPECT_EQ(ranks[7], Rank::RANK_1);
}
