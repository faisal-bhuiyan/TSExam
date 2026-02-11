#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "problem_1/geometry.hpp"
#include "problem_1/reorient_triangles.hpp"
#include "problem_1/stl_io.hpp"
#include "problem_1/triangle_mesh.hpp"

using tsexam::problem1::are_orientations_consistent;
using tsexam::problem1::Edge;
using tsexam::problem1::flip_triangle;
using tsexam::problem1::has_directed_edge;
using tsexam::problem1::make_edge;
using tsexam::problem1::parse_ascii_stl;
using tsexam::problem1::Point;
using tsexam::problem1::PointEquality;
using tsexam::problem1::reorient_inconsistent_triangles;
using tsexam::problem1::Triangle;
using tsexam::problem1::TriangleMesh;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void expect_point_eq(const Point& actual, const Point& expected) {
    EXPECT_DOUBLE_EQ(actual[0], expected[0]);
    EXPECT_DOUBLE_EQ(actual[1], expected[1]);
    EXPECT_DOUBLE_EQ(actual[2], expected[2]);
}

/// Build a TriangleMesh from in-memory STL content (writes to a temp file).
static TriangleMesh make_mesh_from_stl(const std::string& stl_content) {
    const std::string path = "reorient_test.stl";
    std::ofstream f(path);
    if (!f) {
        throw std::runtime_error("failed to open " + path + " for writing");
    }
    f << stl_content;
    f.close();
    return TriangleMesh(path);
}

// ---------------------------------------------------------------------------
// flip_triangle
// ---------------------------------------------------------------------------

TEST(FlipTriangle, SwapsBAndC) {
    Triangle t{{0., 0., 0.}, {1., 0., 0.}, {0., 1., 0.}};
    flip_triangle(t);
    expect_point_eq(t.a, {0., 0., 0.});
    expect_point_eq(t.b, {0., 1., 0.});
    expect_point_eq(t.c, {1., 0., 0.});
}

TEST(FlipTriangle, Idempotent) {
    Triangle t{{0., 0., 0.}, {1., 0., 0.}, {0., 1., 0.}};
    flip_triangle(t);
    flip_triangle(t);
    expect_point_eq(t.a, {0., 0., 0.});
    expect_point_eq(t.b, {1., 0., 0.});
    expect_point_eq(t.c, {0., 1., 0.});
}

// ---------------------------------------------------------------------------
// has_directed_edge
// ---------------------------------------------------------------------------

TEST(HasDirectedEdge, DetectsDirectedEdgesAB_BC_CA) {
    Point a{0., 0., 0.}, b{1., 0., 0.}, c{0., 1., 0.};
    Triangle t{a, b, c};
    EXPECT_TRUE(has_directed_edge(t, a, b));
    EXPECT_TRUE(has_directed_edge(t, b, c));
    EXPECT_TRUE(has_directed_edge(t, c, a));
}

TEST(HasDirectedEdge, RejectsReverseDirection) {
    Point a{0., 0., 0.}, b{1., 0., 0.}, c{0., 1., 0.};
    Triangle t{a, b, c};
    EXPECT_FALSE(has_directed_edge(t, b, a));
    EXPECT_FALSE(has_directed_edge(t, c, b));
    EXPECT_FALSE(has_directed_edge(t, a, c));
}

TEST(HasDirectedEdge, RejectsNonEdge) {
    Point a{0., 0., 0.}, b{1., 0., 0.}, c{0., 1., 0.};
    Triangle t{a, b, c};
    Point d{2., 0., 0.};
    EXPECT_FALSE(has_directed_edge(t, a, d));
}

// ---------------------------------------------------------------------------
// are_orientations_consistent
// ---------------------------------------------------------------------------

TEST(AreOrientationsConsistent, OppositeDirectionIsConsistent) {
    Point p{0., 0., 0.}, q{1., 0., 0.}, r{0., 1., 0.};
    Triangle t1{p, q, r};  // edge (p,q)
    Triangle t2{q, p, r};  // edge (q,p) -> opposite
    Edge e = make_edge(p, q);
    EXPECT_TRUE(are_orientations_consistent(t1, t2, e));
}

TEST(AreOrientationsConsistent, SameDirectionIsInconsistent) {
    Point p{0., 0., 0.}, q{1., 0., 0.}, r1{0., 1., 0.}, r2{1., 1., 0.};
    Triangle t1{p, q, r1};  // edge (p,q)
    Triangle t2{p, q, r2};  // edge (p,q) same direction
    Edge e = make_edge(p, q);
    EXPECT_FALSE(are_orientations_consistent(t1, t2, e));
}

// ---------------------------------------------------------------------------
// reorient_inconsistent_triangles — mesh from STL
// ---------------------------------------------------------------------------

TEST(ReorientInconsistentTriangles, SeedOutOfRangeReturnsEmpty) {
    const std::string stl = R"(
solid one
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0 1 0
    endloop
  endfacet
endsolid one
)";
    TriangleMesh mesh = make_mesh_from_stl(stl);
    auto flipped = reorient_inconsistent_triangles(mesh, 1);
    EXPECT_TRUE(flipped.empty());
}

TEST(ReorientInconsistentTriangles, SingleTriangleReturnsEmpty) {
    const std::string stl = R"(
solid one
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0 1 0
    endloop
  endfacet
endsolid one
)";
    TriangleMesh mesh = make_mesh_from_stl(stl);
    auto flipped = reorient_inconsistent_triangles(mesh, 0);
    EXPECT_TRUE(flipped.empty());
}

TEST(ReorientInconsistentTriangles, TwoTrianglesConsistentReturnsEmpty) {
    // Two triangles sharing edge (0,0,0)-(1,0,0), opposite orientation
    const std::string stl = R"(
solid two_consistent
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0 1 0
    endloop
  endfacet
  facet normal 0 0 -1
    outer loop
      vertex 0 0 0
      vertex 0 1 0
      vertex 1 0 0
    endloop
  endfacet
endsolid two_consistent
)";
    TriangleMesh mesh = make_mesh_from_stl(stl);
    auto flipped = reorient_inconsistent_triangles(mesh, 0);
    EXPECT_TRUE(flipped.empty());
}

TEST(ReorientInconsistentTriangles, TwoTrianglesInconsistentReturnsOneFlipped) {
    // Two triangles sharing edge (0,0,0)-(1,0,0), same orientation -> neighbor flipped
    const std::string stl = R"(
solid two_inconsistent
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0 1 0
    endloop
  endfacet
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 1 1 0
    endloop
  endfacet
endsolid two_inconsistent
)";
    TriangleMesh mesh = make_mesh_from_stl(stl);
    auto flipped = reorient_inconsistent_triangles(mesh, 0);
    ASSERT_EQ(flipped.size(), 1u);
    // Neighbor (index 1) had vertices (0,0,0), (1,0,0), (1,1,0). Flipped -> (0,0,0), (1,1,0),
    // (1,0,0).
    expect_point_eq(flipped[0].a, {0., 0., 0.});
    expect_point_eq(flipped[0].b, {1., 1., 0.});
    expect_point_eq(flipped[0].c, {1., 0., 0.});
}

TEST(ReorientInconsistentTriangles, MeshUnchangedAfterCall) {
    const std::string stl = R"(
solid two_inconsistent
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0 1 0
    endloop
  endfacet
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 1 1 0
    endloop
  endfacet
endsolid two_inconsistent
)";
    TriangleMesh mesh = make_mesh_from_stl(stl);
    const auto& before = mesh.GetTriangles();
    (void)reorient_inconsistent_triangles(mesh, 0);
    const auto& after = mesh.GetTriangles();
    ASSERT_EQ(before.size(), after.size());
    for (std::size_t i = 0; i < before.size(); ++i) {
        expect_point_eq(after[i].a, before[i].a);
        expect_point_eq(after[i].b, before[i].b);
        expect_point_eq(after[i].c, before[i].c);
    }
}

// ---------------------------------------------------------------------------
// export_inconsistent_triangles
// ---------------------------------------------------------------------------

TEST(ExportInconsistentTriangles, WritesValidStlToStreamAndReadsItBack) {
    const std::string stl = R"(
solid two_inconsistent
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0 1 0
    endloop
  endfacet
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 1 1 0
    endloop
  endfacet
endsolid two_inconsistent
)";
    TriangleMesh mesh = make_mesh_from_stl(stl);
    std::ostringstream out;
    export_inconsistent_triangles(mesh, 0, out);
    std::string s = out.str();
    EXPECT_TRUE(s.find("solid") != std::string::npos);
    EXPECT_TRUE(s.find("endsolid") != std::string::npos);
    EXPECT_TRUE(s.find("vertex") != std::string::npos);

    std::istringstream in(s);
    auto triangles = parse_ascii_stl(in);

    ASSERT_EQ(triangles.size(), 1u);
    // Neighbor (index 1) had vertices (0,0,0), (1,0,0), (1,1,0)
    // Flipped -> (0,0,0), (1,1,0), (1,0,0)
    expect_point_eq(triangles[0].a, {0., 0., 0.});
    expect_point_eq(triangles[0].b, {1., 1., 0.});
    expect_point_eq(triangles[0].c, {1., 0., 0.});
}
