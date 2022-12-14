#include "types.h"

#include <gtest/gtest.h>

namespace busrpc { namespace test {

enum class TestEnum { None = 0, E1 = 1, E2 = 2, E3 = 4, All = 7 };
DEFINE_BITWISE_ENUM(TestEnum);

TEST(BitwiseEnumTest, And_Returns_Correct_Result)
{
    TestEnum flags = TestEnum::All;
    TestEnum result = flags & TestEnum::E3;

    EXPECT_EQ(static_cast<std::underlying_type_t<TestEnum>>(result), 4);

    result &= TestEnum::E3;

    EXPECT_EQ(static_cast<std::underlying_type_t<TestEnum>>(result), 4);

    result &= TestEnum::E1;

    EXPECT_EQ(static_cast<std::underlying_type_t<TestEnum>>(result), 0);
}

TEST(BitwiseEnumTest, Or_Returns_Correct_Result)
{
    TestEnum flags = TestEnum::None;
    TestEnum result = flags | TestEnum::E2;

    EXPECT_EQ(static_cast<std::underlying_type_t<TestEnum>>(result), 2);

    result |= TestEnum::E1;

    EXPECT_EQ(static_cast<std::underlying_type_t<TestEnum>>(result), 3);
}

TEST(BitwiseEnumTest, Negation_Returns_Correct_Result)
{
    EXPECT_EQ(static_cast<std::underlying_type_t<TestEnum>>(~TestEnum::None), 7);
    EXPECT_EQ(static_cast<std::underlying_type_t<TestEnum>>(~TestEnum::All), 0);
    EXPECT_EQ(static_cast<std::underlying_type_t<TestEnum>>(~TestEnum::E1), 6);
}

TEST(BitwiseEnumTest, Xor_Returns_Correct_Result)
{
    TestEnum flags = TestEnum::None;
    TestEnum result = flags ^ TestEnum::E2;

    EXPECT_EQ(static_cast<std::underlying_type_t<TestEnum>>(result), 2);

    result ^= TestEnum::E1;

    EXPECT_EQ(static_cast<std::underlying_type_t<TestEnum>>(result), 3);

    result ^= TestEnum::E1;

    EXPECT_EQ(static_cast<std::underlying_type_t<TestEnum>>(result), 2);
}

TEST(BitwiseEnumTest, CheckAny_Returns_Correct_Result)
{
    EXPECT_FALSE(CheckAny(TestEnum::None, TestEnum::E1));
    EXPECT_FALSE(CheckAny(TestEnum::None, TestEnum::All));
    EXPECT_FALSE(CheckAny(TestEnum::E1 | TestEnum::E2, TestEnum::E3));
    EXPECT_FALSE(CheckAny(TestEnum::E1, TestEnum::E2 | TestEnum::E3));
    EXPECT_TRUE(CheckAny(TestEnum::None, TestEnum::None));
    EXPECT_TRUE(CheckAny(TestEnum::All, TestEnum::All));
    EXPECT_TRUE(CheckAny(TestEnum::E1, TestEnum::None));
    EXPECT_TRUE(CheckAny(TestEnum::E1, TestEnum::E1));
    EXPECT_TRUE(CheckAny(TestEnum::E1, TestEnum::All));
    EXPECT_TRUE(CheckAny(TestEnum::E1 | TestEnum::E2, TestEnum::E2));
    EXPECT_TRUE(CheckAny(TestEnum::E1, TestEnum::E3 | TestEnum::E1));
}

TEST(BitwiseEnumTest, CheckAll_Returns_Correct_Result)
{
    EXPECT_FALSE(CheckAll(TestEnum::None, TestEnum::E1));
    EXPECT_FALSE(CheckAll(TestEnum::None, TestEnum::All));
    EXPECT_FALSE(CheckAll(TestEnum::E1 | TestEnum::E2, TestEnum::E3));
    EXPECT_FALSE(CheckAll(TestEnum::E1, TestEnum::E1 | TestEnum::E3));
    EXPECT_FALSE(CheckAll(TestEnum::E1 | TestEnum::E3, TestEnum::All));
    EXPECT_TRUE(CheckAll(TestEnum::None, TestEnum::None));
    EXPECT_TRUE(CheckAll(TestEnum::All, TestEnum::All));
    EXPECT_TRUE(CheckAll(TestEnum::E1, TestEnum::None));
    EXPECT_TRUE(CheckAll(TestEnum::E1, TestEnum::E1));
    EXPECT_TRUE(CheckAll(TestEnum::E1 | TestEnum::E2, TestEnum::E2 | TestEnum::E1));
    EXPECT_TRUE(CheckAll(TestEnum::E1 | TestEnum::E2 | TestEnum::E3, TestEnum::All));
}
}} // namespace busrpc::test
