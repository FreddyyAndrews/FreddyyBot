#include <gtest/gtest.h>
#include "square.h"

TEST(SquareTest, TestIsBetweenRank1)
{
    Square square_a = Square(0, 0);
    Square square_b = Square(0, 6);
    Square test_square = Square(0, 4);
    EXPECT_TRUE(test_square.is_between(square_a, square_b));
}

TEST(SquareTest, TestIsBetweenRank2)
{
    Square square_a = Square(4, 2);
    Square square_b = Square(4, 6);
    Square test_square = Square(4, 3);
    EXPECT_TRUE(test_square.is_between(square_a, square_b));
}

TEST(SquareTest, TestIsBetweenFile1)
{
    Square square_a = Square(6, 3);
    Square square_b = Square(2, 3);
    Square test_square = Square(4, 3);
    EXPECT_TRUE(test_square.is_between(square_a, square_b));
}

TEST(SquareTest, TestIsBetweenFile2)
{
    Square square_a = Square(7, 6);
    Square square_b = Square(2, 6);
    Square test_square = Square(4, 6);
    EXPECT_TRUE(test_square.is_between(square_a, square_b));
}

TEST(SquareTest, TestIsBetweenDiagonal1)
{
    Square square_a = Square(7, 1);
    Square square_b = Square(2, 6);
    Square test_square = Square(4, 4);
    EXPECT_TRUE(test_square.is_between(square_a, square_b));
}

TEST(SquareTest, TestIsBetweenDiagonal2)
{
    Square square_a = Square(6, 5);
    Square square_b = Square(4, 7);
    Square test_square = Square(5, 6);
    EXPECT_TRUE(test_square.is_between(square_a, square_b));
}

TEST(SquareTest, TestNotInBetween1)
{
    Square square_a = Square(5, 6);
    Square square_b = Square(3, 2);
    Square test_square = Square(5, 4);
    EXPECT_FALSE(test_square.is_between(square_a, square_b));
}

TEST(SquareTest, TestNotInBetween2)
{
    Square square_a = Square(1, 2);
    Square square_b = Square(3, 2);
    Square test_square = Square(5, 4);
    EXPECT_FALSE(test_square.is_between(square_a, square_b));
}

TEST(SquareTest, TestNotInBetween3)
{
    Square square_a = Square(1, 2);
    Square square_b = Square(1, 6);
    Square test_square = Square(5, 4);
    EXPECT_FALSE(test_square.is_between(square_a, square_b));
}

TEST(SquareTest, TestDiagonal3)
{
    Square square_a = Square(0, 4);
    Square square_b = Square(4, 0);
    Square test_square = Square(1, 3);
    EXPECT_TRUE(test_square.is_between(square_a, square_b));
}
