#include "pvz/core/Rng.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace pvz {
namespace {

TEST(Rng, SameSeedProducesSameSequence) {
    Mt19937Rng a{42};
    Mt19937Rng b{42};
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(a.nextInt(0, 1000), b.nextInt(0, 1000));
    }
}

TEST(Rng, DifferentSeedsDiverge) {
    Mt19937Rng a{1};
    Mt19937Rng b{2};

    bool anyDifferent = false;
    for (int i = 0; i < 50 && !anyDifferent; ++i) {
        anyDifferent = a.nextInt(0, 1'000'000) != b.nextInt(0, 1'000'000);
    }
    EXPECT_TRUE(anyDifferent);
}

TEST(Rng, NextIntRespectsInclusiveBounds) {
    Mt19937Rng rng{7};
    for (int i = 0; i < 500; ++i) {
        const int value = rng.nextInt(3, 6);
        EXPECT_GE(value, 3);
        EXPECT_LE(value, 6);
    }
}

TEST(Rng, NextIntWithEqualBoundsReturnsThatValue) {
    Mt19937Rng rng{7};
    EXPECT_EQ(rng.nextInt(5, 5), 5);
}

TEST(Rng, NextFloatRespectsBounds) {
    Mt19937Rng rng{11};
    for (int i = 0; i < 500; ++i) {
        const float value = rng.nextFloat(-1.5F, 2.5F);
        EXPECT_GE(value, -1.5F);
        EXPECT_LE(value, 2.5F);
    }
}

TEST(SequenceRngTest, ReplaysValuesInOrderThenCycles) {
    SequenceRng rng{{1, 2, 3}};
    EXPECT_EQ(rng.nextInt(0, 10), 1);
    EXPECT_EQ(rng.nextInt(0, 10), 2);
    EXPECT_EQ(rng.nextInt(0, 10), 3);
    EXPECT_EQ(rng.nextInt(0, 10), 1) << "sequence should wrap";
}

TEST(SequenceRngTest, ClampsToRequestedRange) {
    SequenceRng rng{{99}};
    EXPECT_EQ(rng.nextInt(0, 4), 4);
}

TEST(SequenceRngTest, EmptySequenceIsSafe) {
    SequenceRng rng{{}};
    EXPECT_EQ(rng.nextInt(2, 8), 2);
}

} // namespace
} // namespace pvz
