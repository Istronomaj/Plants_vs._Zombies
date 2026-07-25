#include "pvz/core/GameConfig.hpp"
#include "pvz/core/GridGeometry.hpp"
#include "pvz/core/Rect.hpp"
#include "pvz/core/Vec2.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cmath>

namespace pvz {
namespace {

GridGeometry defaultGeometry() {
    return GameConfig{}.geometry();
}

/// The 45 tile centres the original hardcoded in include/utils.h, in WINDOW
/// space (the map is drawn at 2x). Geometry is stored in map space, so these
/// are halved before comparison.
constexpr std::array<int, 9> kLegacyColumnsX{416, 530, 646, 762, 878, 994, 1110, 1218, 1336};
constexpr std::array<int, 5> kLegacyRowsY{184, 322, 462, 604, 740};
constexpr float kWindowScale = 2.0F;

TEST(Geometry, CellCenterRoundTrip) {
    const GridGeometry geom = defaultGeometry();
    for (int row = 0; row < geom.rows(); ++row) {
        for (int col = 0; col < geom.cols(); ++col) {
            const GridCell cell{row, col};
            const auto found = geom.cellAt(geom.cellCenter(cell));
            ASSERT_TRUE(found.has_value()) << "row=" << row << " col=" << col;
            EXPECT_EQ(*found, cell);
        }
    }
}

/// The uniform grid replaces 45 hand-typed coordinates. It must land close
/// enough to the originals that the lawn art still lines up.
///
/// The legacy columns were not evenly spaced -- the gap from x=1110 to x=1218
/// is 108px where every other gap is 114-116px -- so no uniform grid can
/// reproduce all nine exactly. The worst case is 4px on column 6, i.e. 3.5% of
/// a cell, which is invisible and well inside the 120px click box.
TEST(Geometry, MatchesLegacyHardcodedCentres) {
    const GridGeometry geom = defaultGeometry();
    constexpr float kToleranceWindowPx = 4.5F;

    for (int row = 0; row < geom.rows(); ++row) {
        for (int col = 0; col < geom.cols(); ++col) {
            const Vec2 center = geom.cellCenter({row, col});
            const float legacyX = static_cast<float>(kLegacyColumnsX[static_cast<std::size_t>(col)]);
            const float legacyY = static_cast<float>(kLegacyRowsY[static_cast<std::size_t>(row)]);

            EXPECT_LT(std::abs(center.x * kWindowScale - legacyX), kToleranceWindowPx)
                << "column " << col;
            EXPECT_LT(std::abs(center.y * kWindowScale - legacyY), kToleranceWindowPx)
                << "row " << row;
        }
    }
}

/// A click just left of or above the lawn must not truncate onto cell 0.
TEST(Geometry, OutOfBoundsReturnsNullopt) {
    const GridGeometry geom = defaultGeometry();
    const Vec2 origin = geom.origin();
    const Vec2 cell = geom.cellSize();

    EXPECT_FALSE(geom.cellAt({origin.x - cell.x, origin.y}).has_value()) << "left of lawn";
    EXPECT_FALSE(geom.cellAt({origin.x, origin.y - cell.y}).has_value()) << "above lawn";
    EXPECT_FALSE(geom.cellAt({origin.x + cell.x * static_cast<float>(geom.cols()), origin.y})
                     .has_value())
        << "right of lawn";
    EXPECT_FALSE(geom.cellAt({origin.x, origin.y + cell.y * static_cast<float>(geom.rows())})
                     .has_value())
        << "below lawn";
}

TEST(Geometry, LaneYMatchesCellCenterY) {
    const GridGeometry geom = defaultGeometry();
    for (int row = 0; row < geom.rows(); ++row) {
        EXPECT_FLOAT_EQ(geom.laneY(row), geom.cellCenter({row, 0}).y);
    }
}

/// Zombies walk right-to-left, so the house must be left of the spawn line.
TEST(Geometry, HouseIsLeftOfSpawn) {
    const GridGeometry geom = defaultGeometry();
    EXPECT_LT(geom.houseX(), geom.origin().x);
    EXPECT_GT(geom.spawnX(), geom.cellCenter({0, geom.cols() - 1}).x);
}

TEST(RectMath, FromCenterIsCenteredOnPoint) {
    const Rect rect = Rect::fromCenter({100.0F, 50.0F}, {30.0F, 10.0F});
    EXPECT_FLOAT_EQ(rect.center().x, 100.0F);
    EXPECT_FLOAT_EQ(rect.center().y, 50.0F);
    EXPECT_FLOAT_EQ(rect.left(), 85.0F);
    EXPECT_FLOAT_EQ(rect.top(), 45.0F);
}

TEST(RectMath, IntersectsIsSymmetricAndExcludesTouching) {
    const Rect a = Rect::fromCenter({0.0F, 0.0F}, {10.0F, 10.0F});
    const Rect b = Rect::fromCenter({5.0F, 0.0F}, {10.0F, 10.0F});
    const Rect touching = Rect::fromCenter({10.0F, 0.0F}, {10.0F, 10.0F});
    const Rect apart = Rect::fromCenter({100.0F, 0.0F}, {10.0F, 10.0F});

    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(b.intersects(a));
    EXPECT_FALSE(a.intersects(touching)) << "edge-touching rects must not count as overlapping";
    EXPECT_FALSE(a.intersects(apart));
}

TEST(RectMath, ContainsUsesHalfOpenBounds) {
    const Rect rect{{0.0F, 0.0F}, {10.0F, 10.0F}};
    EXPECT_TRUE(rect.contains({0.0F, 0.0F}));
    EXPECT_TRUE(rect.contains({9.99F, 9.99F}));
    EXPECT_FALSE(rect.contains({10.0F, 5.0F}));
    EXPECT_FALSE(rect.contains({-0.01F, 5.0F}));
}

TEST(Vec2Math, NormalizedOfZeroIsZero) {
    EXPECT_EQ(normalized(Vec2{}), Vec2{});
}

TEST(Vec2Math, NormalizedHasUnitLength) {
    const Vec2 unit = normalized({3.0F, 4.0F});
    EXPECT_NEAR(length(unit), 1.0F, 1e-5F);
    EXPECT_NEAR(unit.x, 0.6F, 1e-5F);
    EXPECT_NEAR(unit.y, 0.8F, 1e-5F);
}

} // namespace
} // namespace pvz
