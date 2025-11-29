#include <gtest/gtest.h>

#include "utils/enum_flags.hpp"

enum class MyEnum {
    NONE = 0,
    FLAG1 = 1 << 0,
    FLAG2 = 1 << 1,
    FLAG3 = 1 << 2,
};

template<>
struct utils::enable_bitmask_operators<MyEnum> {
    static constexpr bool value = true;
};

TEST(EnumFlags, BitwiseOr) {
    MyEnum flags = MyEnum::FLAG1 | MyEnum::FLAG2;
    EXPECT_EQ(static_cast<int>(flags), 3);
}

TEST(EnumFlags, BitwiseAnd) {
    MyEnum flags = MyEnum::FLAG1 | MyEnum::FLAG2;
    EXPECT_EQ(flags & MyEnum::FLAG1, MyEnum::FLAG1);
    EXPECT_EQ(flags & MyEnum::FLAG2, MyEnum::FLAG2);
    EXPECT_EQ(flags & MyEnum::FLAG3, MyEnum::NONE);
}

TEST(EnumFlags, BitwiseXor) {
    MyEnum flags = MyEnum::FLAG1 | MyEnum::FLAG2;
    flags = flags ^ MyEnum::FLAG1;
    EXPECT_EQ(flags, MyEnum::FLAG2);
}

TEST(EnumFlags, BitwiseNot) {
    MyEnum flags = MyEnum::FLAG1;
    flags = ~flags;
    EXPECT_EQ(static_cast<int>(flags), ~1);
}

TEST(EnumFlags, BitwiseOrAssignment) {
    MyEnum flags = MyEnum::FLAG1;
    flags |= MyEnum::FLAG2;
    EXPECT_EQ(static_cast<int>(flags), 3);
}

TEST(EnumFlags, BitwiseAndAssignment) {
    MyEnum flags = MyEnum::FLAG1 | MyEnum::FLAG2;
    flags &= MyEnum::FLAG1;
    EXPECT_EQ(flags, MyEnum::FLAG1);
}

TEST(EnumFlags, BitwiseXorAssignment) {
    MyEnum flags = MyEnum::FLAG1 | MyEnum::FLAG2;
    flags ^= MyEnum::FLAG1;
    EXPECT_EQ(flags, MyEnum::FLAG2);
}

TEST(EnumFlags, DisabledEnum) {
    enum class DisabledEnum { A, B, C };
    // This should not compile if the operators were enabled for all enums
    SUCCEED();
}
