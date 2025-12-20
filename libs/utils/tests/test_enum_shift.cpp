#include <gtest/gtest.h>

#include <wuttang/utils/enum_shift.hpp>

using namespace wuttang::utils;

enum class MyShiftableEnum { A, B, C, D, E, INVALID };

template<>
struct wuttang::utils::EnumTraits<MyShiftableEnum> {
    using ArithmeticType = int;
    static constexpr MyShiftableEnum min = MyShiftableEnum::A;
    static constexpr MyShiftableEnum max = MyShiftableEnum::E;
    static constexpr MyShiftableEnum sentinel = MyShiftableEnum::INVALID;
};

TEST(EnumShiftTest, PositiveShift) {
    EXPECT_EQ(shift(MyShiftableEnum::A, 2), MyShiftableEnum::C);
    EXPECT_EQ(shift(MyShiftableEnum::C, 2), MyShiftableEnum::E);
}

TEST(EnumShiftTest, NegativeShift) {
    EXPECT_EQ(shift(MyShiftableEnum::E, -2), MyShiftableEnum::C);
    EXPECT_EQ(shift(MyShiftableEnum::C, -2), MyShiftableEnum::A);
}

TEST(EnumShiftTest, ShiftToBoundaries) {
    EXPECT_EQ(shift(MyShiftableEnum::A, 4), MyShiftableEnum::E);
    EXPECT_EQ(shift(MyShiftableEnum::E, -4), MyShiftableEnum::A);
}

TEST(EnumShiftTest, ShiftBeyondBoundaries) {
    EXPECT_EQ(shift(MyShiftableEnum::D, 2), MyShiftableEnum::INVALID);
    EXPECT_EQ(shift(MyShiftableEnum::B, -3), MyShiftableEnum::INVALID);
}

TEST(EnumShiftTest, ZeroShift) {
    EXPECT_EQ(shift(MyShiftableEnum::B, 0), MyShiftableEnum::B);
}
