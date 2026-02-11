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

//---------------------------------------------------------------------------
// Helpers
//---------------------------------------------------------------------------

static void expect_point_eq(const Point& actual, const Point& expected) {
    EXPECT_DOUBLE_EQ(actual[0], expected[0]);
    EXPECT_DOUBLE_EQ(actual[1], expected[1]);
    EXPECT_DOUBLE_EQ(actual[2], expected[2]);
}

/// Build a TriangleMesh from in-memory STL content (writes to a temp file)
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

/// Build STL string for a strip of 2 * num_quads triangles (each quad = 2 triangles, inconsistent)
static std::string make_stl_strip_inconsistent(std::size_t num_quads) {
    std::ostringstream stl;
    stl << "solid strip\n";
    for (std::size_t k = 0; k < num_quads; ++k) {
        double x0 = static_cast<double>(k);
        double x1 = static_cast<double>(k + 1);
        // First triangle of quad: (x0,0,0), (x1,0,0), (x0,1,0)
        stl << "  facet normal 0 0 1\n    outer loop\n"
            << "      vertex " << x0 << " 0 0\n"
            << "      vertex " << x1 << " 0 0\n"
            << "      vertex " << x0 << " 1 0\n"
            << "    endloop\n  endfacet\n";
        // Second triangle: (x1,0,0), (x0,1,0), (x1,1,0) — same orientation on shared edge ->
        // inconsistent
        stl << "  facet normal 0 0 1\n    outer loop\n"
            << "      vertex " << x1 << " 0 0\n"
            << "      vertex " << x0 << " 1 0\n"
            << "      vertex " << x1 << " 1 0\n"
            << "    endloop\n  endfacet\n";
    }
    stl << "endsolid strip\n";
    return stl.str();
}

/// Build STL string for an NxM grid of triangles, all consistently oriented
static std::string make_stl_grid_consistent(std::size_t nrows, std::size_t ncols) {
    std::ostringstream stl;
    stl << "solid grid\n";
    for (std::size_t i = 0; i < nrows; ++i) {
        for (std::size_t j = 0; j < ncols; ++j) {
            double x0 = static_cast<double>(j);
            double x1 = static_cast<double>(j + 1);
            double y0 = static_cast<double>(i);
            double y1 = static_cast<double>(i + 1);
            // Two triangles per cell, both same winding (e.g. ccw)
            stl << "  facet normal 0 0 1\n    outer loop\n"
                << "      vertex " << x0 << " " << y0 << " 0\n"
                << "      vertex " << x1 << " " << y0 << " 0\n"
                << "      vertex " << x0 << " " << y1 << " 0\n"
                << "    endloop\n  endfacet\n";
            stl << "  facet normal 0 0 1\n    outer loop\n"
                << "      vertex " << x1 << " " << y0 << " 0\n"
                << "      vertex " << x1 << " " << y1 << " 0\n"
                << "      vertex " << x0 << " " << y1 << " 0\n"
                << "    endloop\n  endfacet\n";
        }
    }
    stl << "endsolid grid\n";
    return stl.str();
}

/// Strip facets with y offset (for building disconnected multi-strip STL)
static std::string make_stl_strip_facets_at_y(std::size_t num_quads, double y_offset) {
    std::ostringstream stl;
    double y0 = y_offset;
    double y1 = y_offset + 1.0;
    for (std::size_t k = 0; k < num_quads; ++k) {
        double x0 = static_cast<double>(k);
        double x1 = static_cast<double>(k + 1);
        stl << "  facet normal 0 0 1\n    outer loop\n"
            << "      vertex " << x0 << " " << y0 << " 0\n"
            << "      vertex " << x1 << " " << y0 << " 0\n"
            << "      vertex " << x0 << " " << y1 << " 0\n"
            << "    endloop\n  endfacet\n";
        stl << "  facet normal 0 0 1\n    outer loop\n"
            << "      vertex " << x1 << " " << y0 << " 0\n"
            << "      vertex " << x0 << " " << y1 << " 0\n"
            << "      vertex " << x1 << " " << y1 << " 0\n"
            << "    endloop\n  endfacet\n";
    }
    return stl.str();
}

/// Build STL string for an NxM grid of triangles, all inconsistently oriented.
/// Stacked strips so every shared edge has the same direction in both triangles.
static std::string make_stl_grid_inconsistent(std::size_t nrows, std::size_t ncols) {
    std::ostringstream stl;
    stl << "solid grid_inconsistent\n";
    for (std::size_t i = 0; i < nrows; ++i) {
        stl << make_stl_strip_facets_at_y(ncols, static_cast<double>(i));
    }
    stl << "endsolid grid_inconsistent\n";
    return stl.str();
}

//---------------------------------------------------------------------------
// flip_triangle
//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
// has_directed_edge
//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
// are_orientations_consistent
//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
// reorient_inconsistent_triangles — mesh from STL
//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
// export_inconsistent_triangles
//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
// Stress tests — time and space
//---------------------------------------------------------------------------

TEST(Stress, LargeConsistentGrid_ReturnsEmpty) {
    // Grid of 50x40 = 2000 cells -> 4000 triangles; all consistent
    const std::size_t rows = 50;
    const std::size_t cols = 40;
    std::string stl = make_stl_grid_consistent(rows, cols);
    TriangleMesh mesh = make_mesh_from_stl(stl);
    ASSERT_EQ(mesh.GetTriangles().size(), rows * cols * 2u);

    auto flipped = reorient_inconsistent_triangles(mesh, 0);
    EXPECT_TRUE(flipped.empty());
}

TEST(Stress, LargeInconsistentGrid_ReturnsAllButSeed) {
    // Grid of 50x40 = 2000 cells -> 4000 triangles; all inconsistent (stacked strips)
    const std::size_t rows = 50;
    const std::size_t cols = 40;
    std::string stl = make_stl_grid_inconsistent(rows, cols);
    TriangleMesh mesh = make_mesh_from_stl(stl);
    ASSERT_EQ(mesh.GetTriangles().size(), rows * cols * 2u);

    auto flipped = reorient_inconsistent_triangles(mesh, 0);
    EXPECT_EQ(flipped.size(), rows * cols * 2u - 1u);

    {
        std::ofstream out("stress _grid_reoriented.stl");
        if (out) {
            write_ascii_stl(out, "stress_grid_reoriented", flipped);
        }
    }
}

TEST(Stress, LongStripAllInconsistent_ReturnsAllButSeed) {
    // Strip of 500 quads -> 1000 triangles; each pair inconsistent
    const std::size_t num_quads = 500;
    std::string stl = make_stl_strip_inconsistent(num_quads);
    TriangleMesh mesh = make_mesh_from_stl(stl);
    ASSERT_EQ(mesh.GetTriangles().size(), num_quads * 2u);

    auto flipped = reorient_inconsistent_triangles(mesh, 0);
    EXPECT_EQ(flipped.size(), num_quads * 2u - 1u);

    {
        std::ofstream out("stress _strip_reoriented.stl");
        if (out) {
            write_ascii_stl(out, "stress_strip_reoriented", flipped);
        }
    }
}

TEST(Stress, TwoDisconnectedComponents_OnlySeedComponentProcessed) {
    // Two strips: first 3 quads (6 tri), second 2 quads (4 tri). Disconnected.
    std::string stl1 = make_stl_strip_inconsistent(3);
    std::string stl2 = make_stl_strip_inconsistent(2);

    // Offset second strip so it doesn't touch the first (e.g. y += 10)
    std::ostringstream combined;
    combined << "solid two\n"
             << stl1.substr(stl1.find("facet"), stl1.rfind("endfacet") - stl1.find("facet") + 9);
    combined << "  facet normal 0 0 1\n    outer loop\n      vertex 0 10 0\n      vertex 1 10 0\n   "
                "   vertex 0 11 0\n    endloop\n  endfacet\n";
    combined << "  facet normal 0 0 1\n    outer loop\n      vertex 1 10 0\n      vertex 0 11 0\n   "
                "   vertex 1 11 0\n    endloop\n  endfacet\n";
    combined << "  facet normal 0 0 1\n    outer loop\n      vertex 0 11 0\n      vertex 1 11 0\n   "
                "   vertex 0 12 0\n    endloop\n  endfacet\n";
    combined << "  facet normal 0 0 1\n    outer loop\n      vertex 1 11 0\n      vertex 0 12 0\n   "
                "   vertex 1 12 0\n    endloop\n  endfacet\n";
    combined << "endsolid two\n";
    TriangleMesh mesh = make_mesh_from_stl(combined.str());

    // Seed 0 in first component (6 triangles). Second component has 4 triangles, unreachable.
    auto flipped = reorient_inconsistent_triangles(mesh, 0);

    // First component: 6 triangles, 5 flipped (all but seed)
    EXPECT_EQ(flipped.size(), 5u);

    {
        std::ofstream out("stress _strip_reoriented.stl");
        if (out) {
            write_ascii_stl(out, "stress_strip_reoriented", flipped);
        }
    }
}

TEST(Stress, FiveDisconnectedComponents_OnlySeedComponentProcessed) {
    // 5 strips at y = 0, 10, 20, 30, 40. Sizes: 2, 2, 100, 2, 2 quads (4, 4, 200, 4, 4 triangles).
    // Seed in the large one (third component); only its 199 flipped triangles are returned.
    const std::size_t small_quads = 2;
    const std::size_t large_quads = 100;
    std::ostringstream combined;
    combined << "solid five\n";
    combined << make_stl_strip_facets_at_y(small_quads, 0.0);   // 4 tri
    combined << make_stl_strip_facets_at_y(small_quads, 10.0);  // 4 tri
    combined << make_stl_strip_facets_at_y(large_quads, 20.0);  // 200 tri (seed component)
    combined << make_stl_strip_facets_at_y(small_quads, 30.0);  // 4 tri
    combined << make_stl_strip_facets_at_y(small_quads, 40.0);  // 4 tri
    combined << "endsolid five\n";

    TriangleMesh mesh = make_mesh_from_stl(combined.str());
    const std::size_t total_tri = 4u + 4u + 200u + 4u + 4u;
    ASSERT_EQ(mesh.GetTriangles().size(), total_tri);

    // Seed = first triangle of the large (third) component: 4 + 4 = 8
    const std::size_t seed = 4u + 4u;
    auto flipped = reorient_inconsistent_triangles(mesh, seed);

    // Only the large component is processed: 200 - 1 = 199 flipped
    EXPECT_EQ(flipped.size(), large_quads * 2u - 1u);

    {
        std::ofstream out("stress _strip_reoriented.stl");
        if (out) {
            write_ascii_stl(out, "stress_strip_reoriented", flipped);
        }
    }
}
