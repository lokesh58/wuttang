#include <gtest/gtest.h>

#include <ranges>
#include <vector>

#include "utils/enum_range.hpp"

enum class TestEnum { A, B, C, D, E };

TEST(EnumRange, ForwardIteration) {
    std::vector<TestEnum> expected = {TestEnum::B, TestEnum::C, TestEnum::D};
    std::vector<TestEnum> actual;
    for (auto e : utils::EnumRange<TestEnum, TestEnum::B, TestEnum::D>()) {
        actual.push_back(e);
    }
    EXPECT_EQ(actual, expected);
}

TEST(EnumRange, ReverseIteration) {
    std::vector<TestEnum> expected = {TestEnum::D, TestEnum::C, TestEnum::B};
    std::vector<TestEnum> actual;
    auto range = utils::EnumRange<TestEnum, TestEnum::B, TestEnum::D>();
    for (auto e : std::views::reverse(range)) {
        actual.push_back(e);
    }
    EXPECT_EQ(actual, expected);
}

TEST(EnumRange, SingleElementRange) {
    std::vector<TestEnum> expected = {TestEnum::C};
    std::vector<TestEnum> actual;
    for (auto e : utils::EnumRange<TestEnum, TestEnum::C, TestEnum::C>()) {
        actual.push_back(e);
    }
    EXPECT_EQ(actual, expected);
}

TEST(EnumRange, EmptyRange) {
    std::vector<TestEnum> actual;
    for (auto e : utils::EnumRange<TestEnum, TestEnum::D, TestEnum::B>()) {
        actual.push_back(e);
    }
    EXPECT_TRUE(actual.empty());
}
