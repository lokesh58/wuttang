#include <gtest/gtest.h>

#include <wuttang/chess.hpp>

using namespace wuttang::chess;

TEST(MoveListTest, InitialState) {
    MoveList move_list;
    EXPECT_EQ(move_list.size(), 0);
    EXPECT_TRUE(move_list.empty());
}

TEST(MoveListTest, PushBackAndSize) {
    MoveList move_list;
    move_list.push_back(Move{});
    EXPECT_EQ(move_list.size(), 1);
    EXPECT_FALSE(move_list.empty());
}

TEST(MoveListTest, PopBack) {
    MoveList move_list;
    move_list.push_back(Move{});
    move_list.push_back(Move{});

    move_list.pop_back();
    EXPECT_EQ(move_list.size(), 1);
}

TEST(MoveListTest, Clear) {
    MoveList move_list;
    move_list.push_back(Move{});
    move_list.clear();
    EXPECT_EQ(move_list.size(), 0);
    EXPECT_TRUE(move_list.empty());
}

TEST(MoveListTest, Iterators) {
    MoveList move_list;
    move_list.push_back(Move{});
    move_list.push_back(Move{});

    int count = 0;
    for (const auto& move : move_list) {
        (void) move;
        ++count;
    }
    EXPECT_EQ(count, 2);
}

TEST(MoveListTest, RandomAccess) {
    MoveList move_list;
    auto move1 = Move::promotion(Square::A7, Square::A8, Piece::WHITE_KNIGHT);
    auto move2 = Move::quiet(Square::H2, Square::H1);

    move_list.push_back(move1);
    move_list.push_back(move2);

    EXPECT_EQ(move_list[0], move1);
    EXPECT_EQ(move_list[1], move2);
}
