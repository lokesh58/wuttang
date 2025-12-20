#include <gtest/gtest.h>

#include <wuttang/chess.hpp>

using namespace wuttang::chess;

TEST(ColorTest, Invert) {
    EXPECT_EQ(invert(Color::WHITE), Color::BLACK);
    EXPECT_EQ(invert(Color::BLACK), Color::WHITE);
}
