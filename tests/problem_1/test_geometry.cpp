#include <sstream>
#include <string>
#include <unordered_map>

#include <gtest/gtest.h>

#include "problem_1/geometry.hpp"

using tsexam::problem1::Edge;
using tsexam::problem1::EdgeEquality;
using tsexam::problem1::EdgeHash;
using tsexam::problem1::make_edge;
using tsexam::problem1::Point;
using tsexam::problem1::PointEquality;
using tsexam::problem1::PointHash;
using tsexam::problem1::Triangle;

// ---------------------------------------------------------------------------
// Point hashing and equality
// ---------------------------------------------------------------------------

TEST(PointHash, IdenticalPointsHaveSameHash) {
    Point p1{1., 2., 3.};
    Point p2{1., 2., 3.};

    PointHash hash;
    EXPECT_EQ(hash(p1), hash(p2));
}

TEST(PointEquality, IdenticalPointsAreEqual) {
    Point p1{1., 2., 3.};
    Point p2{1., 2., 3.};

    PointEquality eq;
    EXPECT_TRUE(eq(p1, p2));
}

TEST(PointEquality, DifferentPointsAreNotEqual) {
    Point p1{1., 2., 3.};
    Point p2{1., 2., 4.};

    PointEquality eq;
    EXPECT_FALSE(eq(p1, p2));
}

// ---------------------------------------------------------------------------
// Edge canonicalization
// ---------------------------------------------------------------------------

TEST(MakeEdge, CanonicalizesOrder) {
    Point p{0., 0., 0.};
    Point q{1., 0., 0.};

    Edge e1 = make_edge(p, q);
    Edge e2 = make_edge(q, p);

    EXPECT_TRUE(PointEquality{}(e1.first, e2.first));
    EXPECT_TRUE(PointEquality{}(e1.second, e2.second));
}

// ---------------------------------------------------------------------------
// Edge hashing and equality
// ---------------------------------------------------------------------------

TEST(EdgeHash, IdenticalEdgesHaveSameHash) {
    Point p{0., 0., 0.};
    Point q{1., 0., 0.};

    Edge e1 = make_edge(p, q);
    Edge e2 = make_edge(q, p);

    EdgeHash hash;
    EXPECT_EQ(hash(e1), hash(e2));
}

TEST(EdgeEquality, IdenticalEdgesAreEqual) {
    Point p{0., 0., 0.};
    Point q{1., 0., 0.};

    Edge e1 = make_edge(p, q);
    Edge e2 = make_edge(q, p);

    EdgeEquality eq;
    EXPECT_TRUE(eq(e1, e2));
}

TEST(EdgeEquality, DifferentEdgesAreNotEqual) {
    Point p{0., 0., 0.};
    Point q{1., 0., 0.};
    Point r{0., 1., 0.};

    Edge e1 = make_edge(p, q);
    Edge e2 = make_edge(p, r);

    EdgeEquality eq;
    EXPECT_FALSE(eq(e1, e2));
}

// ---------------------------------------------------------------------------
// Unordered map behavior
// ---------------------------------------------------------------------------

TEST(EdgeHash, WorksInUnorderedMap) {
    std::unordered_map<Edge, int, EdgeHash, EdgeEquality> map;

    Point p{0., 0., 0.};
    Point q{1., 0., 0.};

    Edge e1 = make_edge(p, q);
    Edge e2 = make_edge(q, p);

    map[e1] = 42;

    EXPECT_EQ(map.size(), 1u);
    EXPECT_EQ(map[e2], 42);
}

// ---------------------------------------------------------------------------
// Triangle structure
// ---------------------------------------------------------------------------

TEST(Triangle, StoresVerticesCorrectly) {
    Triangle t{{0., 0., 0.}, {1., 0., 0.}, {0., 1., 0.}};

    EXPECT_EQ(t.a, (Point{0., 0., 0.}));
    EXPECT_EQ(t.b, (Point{1., 0., 0.}));
    EXPECT_EQ(t.c, (Point{0., 1., 0.}));
}
