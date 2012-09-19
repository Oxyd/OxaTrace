#include "image.hpp"
#include <gtest/gtest.h>

using namespace oxatrace;

TEST(channel_test, construction_clamps) {
  channel c1;
  EXPECT_DOUBLE_EQ(0.0, c1.get());

  channel c2(0.5);
  EXPECT_DOUBLE_EQ(0.5, c2.get());

  channel c3(1.2);
  EXPECT_DOUBLE_EQ(1.0, c3.get());

  channel c4(0.0);
  EXPECT_DOUBLE_EQ(0.0, c4.get());

  channel c5(-0.2);
  EXPECT_DOUBLE_EQ(0.0, c5.get());
}

TEST(channel_test, assignment_clamps) {
  channel c;
  c = 0.5;
  EXPECT_DOUBLE_EQ(0.5, c.get());

  c = 1.2;
  EXPECT_DOUBLE_EQ(1.0, c.get());

  c = -1.94;
  EXPECT_DOUBLE_EQ(0.0, c.get());
}

TEST(channel_test, convertible_to_double) {
  channel c(0.3);
  double d = c;

  EXPECT_DOUBLE_EQ(0.3, d);
}

TEST(channel_test, addition_clamps) {
  channel a(0.3), b(0.4), c(0.8);
  channel d = a + b;
  EXPECT_DOUBLE_EQ(0.7, d.get());

  channel e = a + c;
  EXPECT_DOUBLE_EQ(1.0, e.get());

  channel f = a;
  f += b;
  EXPECT_DOUBLE_EQ(0.7, f.get());

  a += c;
  EXPECT_DOUBLE_EQ(1.0, a.get());
}

TEST(channel_test, subtraction_clamps) {
  channel a(0.3), b(0.4);
  channel c = b - a;
  EXPECT_DOUBLE_EQ(0.1, c.get());

  channel d = a - b;
  EXPECT_DOUBLE_EQ(0.0, d.get());

  channel e = b;
  e -= a;
  EXPECT_DOUBLE_EQ(0.1, e.get());

  a -= b;
  EXPECT_DOUBLE_EQ(0.0, a.get());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
