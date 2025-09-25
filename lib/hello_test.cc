#include "lib/hello.h"
#include "gtest/gtest.h"

namespace scout
{
    // Test fixture for the add function
    TEST(AddTest, HandlesPositiveNumbers)
    {
        EXPECT_EQ(add(2, 3), 5);
    }

    TEST(AddTest, HandlesZero)
    {
        EXPECT_EQ(add(5, 0), 5);
    }

    TEST(AddTest, HandlesNegativeNumbers)
    {
        EXPECT_EQ(add(-2, -3), -5);
        EXPECT_EQ(add(-5, 5), 0);
    }

    TEST(InferTest, Generic)
    {
        EXPECT_EQ(infer(), 561);
    }

}