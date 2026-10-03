#include <gtest/gtest.h>

#include "engine/objects.hxx"
#include "engine/utilities.hxx"

TEST(ATypeTest, Dummy)
{
    EXPECT_EQ(7*6, 42);
}

struct simple_collider : i_collider
{
    explicit simple_collider(coordinate pos, coordinate sz)
        : position(pos)
        , size(sz)
    {
    }

    bounding_box get_bounding_box() const override
    {
        return bounding_box { position, size };
    }
    void on_collision(i_collider* other) override {}

    coordinate position { 0, 0 };
    coordinate size { 1, 1 };
};

TEST(ATypeCollisions, ClearedX)
{
    simple_collider a { {4, 2}, {1, 1} };
    simple_collider b { {3, 2}, {1, 1} };
    
    EXPECT_FALSE( is_collision(a, b) );
}

TEST(ATypeCollisions, ClearedY)
{
    simple_collider a { {4, 2}, {1, 1} };
    simple_collider b { {4, 3}, {1, 1} };
    
    EXPECT_FALSE( is_collision(a, b) );
}

TEST(ATypeCollisions, Cleared)
{
    simple_collider a { {4, 2}, {1, 1} };
    simple_collider b { {5, 3}, {1, 1} };
    
    EXPECT_FALSE( is_collision(a, b) );
}

TEST(ATypeCollisions, CollisionX)
{
    simple_collider a { {4, 2}, {2, 1} };
    simple_collider b { {5, 2}, {1, 1} };
    
    EXPECT_TRUE( is_collision(a, b) );
}

TEST(ATypeCollisions, CollisionY)
{
    simple_collider a { {4, 2}, {1, 2} };
    simple_collider b { {4, 3}, {1, 1} };
    
    EXPECT_TRUE( is_collision(a, b) );
}

TEST(ATypeCollisions, CollisionXY)
{
    simple_collider a { {4, 2}, {2, 2} };
    simple_collider b { {5, 3}, {1, 1} };
    
    EXPECT_TRUE( is_collision(a, b) );
}

TEST(ATypeCollisions, NoSelfCollision)
{
    simple_collider a { {4, 2}, {2, 1} };
    
    EXPECT_FALSE( is_collision(a, a) );
}

TEST(ATypeCoordinates, Equality)
{
    coordinate a { 42, 43 };
    coordinate b { 42, 43 };
    coordinate c { 12, 34 };

    EXPECT_TRUE( (a == b) && (a != c) );
}

TEST(ATypeCoordinates, Addition)
{
    coordinate a { 5, 93 };
    coordinate b { 7, -81 };

    a += b;

    EXPECT_TRUE( (a.x == 12) && (a.y == 12));
}
