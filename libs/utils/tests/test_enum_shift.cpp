#include <gtest/gtest.h>

#include "utils/enum_shift.hpp"

enum class MyShiftableEnum { A, B, C, D, E, INVALID };

template<>
struct utils::EnumTraits<MyShiftableEnum> {
    using ArithmeticType = int;
    static constexpr MyShiftableEnum min = MyShiftableEnum::A;
    static constexpr MyShiftableEnum max = MyShiftableEnum::E;
    static constexpr MyShiftableEnum sentinel = MyShiftableEnum::INVALID;
};

TEST(EnumShift, PositiveShift) {
    EXPECT_EQ(utils::shift(MyShiftableEnum::A, 2), MyShiftableEnum::C);
    EXPECT_EQ(utils::shift(MyShiftableEnum::C, 2), MyShiftableEnum::E);
}

TEST(EnumShift, NegativeShift) {
    EXPECT_EQ(utils::shift(MyShiftableEnum::E, -2), MyShiftableEnum::C);
    EXPECT_EQ(utils::shift(MyShiftableEnum::C, -2), MyShiftableEnum::A);
}

TEST(EnumShift, ShiftToBoundaries) {
    EXPECT_EQ(utils::shift(MyShiftableEnum::A, 4), MyShiftableEnum::E);
    EXPECT_EQ(utils::shift(MyShiftableEnum::E, -4), MyShiftableEnum::A);
}

TEST(EnumShift, ShiftBeyondBoundaries) {
    EXPECT_EQ(utils::shift(MyShiftableEnum::D, 2), MyShiftableEnum::INVALID);
    EXPECT_EQ(utils::shift(MyShiftableEnum::B, -3), MyShiftableEnum::INVALID);
}

TEST(EnumShift, ZeroShift) {
    EXPECT_EQ(utils::shift(MyShiftableEnum::B, 0), MyShiftableEnum::B);
}
