#include <gtest/gtest.h>

#include "chess/color.hpp"

TEST(ColorTest, Invert) {
    EXPECT_EQ(chess::invert(chess::Color::WHITE), chess::Color::BLACK);
    EXPECT_EQ(chess::invert(chess::Color::BLACK), chess::Color::WHITE);
    EXPECT_EQ(chess::invert(chess::Color::BOTH), chess::Color::NONE);
    EXPECT_EQ(chess::invert(chess::Color::NONE), chess::Color::BOTH);
}
