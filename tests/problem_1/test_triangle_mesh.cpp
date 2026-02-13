#include <cmath>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "problem_1/geometry.hpp"
#include "problem_1/triangle_mesh.hpp"

using tsexam::problem1::Edge;
using tsexam::problem1::kBoundaryTriangleIndex;
using tsexam::problem1::make_edge;
using tsexam::problem1::Point;
using tsexam::problem1::Triangle;
using tsexam::problem1::TriangleMesh;

//---------------------------------------------------------------------------
// Helpers
//---------------------------------------------------------------------------

static const char* kTestStlPath = "triangle_mesh_test.stl";

/// Build a TriangleMesh from in-memory STL content (writes to a temp file)
static TriangleMesh make_mesh_from_stl(const std::string& stl_content) {
    std::ofstream f(kTestStlPath);
    if (!f) {
        throw std::runtime_error("failed to open " + std::string(kTestStlPath) + " for writing");
    }
    f << stl_content;
    f.close();
    return TriangleMesh(kTestStlPath);
}

static void expect_point_eq(const Point& actual, const Point& expected) {
    EXPECT_DOUBLE_EQ(actual[0], expected[0]);
    EXPECT_DOUBLE_EQ(actual[1], expected[1]);
    EXPECT_DOUBLE_EQ(actual[2], expected[2]);
}

//---------------------------------------------------------------------------
// Constructor — file open failure
//---------------------------------------------------------------------------

TEST(TriangleMeshConstructor, NonexistentPathThrows) {
    EXPECT_THROW(
        { TriangleMesh mesh("nonexistent_file_that_does_not_exist.stl"); }, std::invalid_argument
    );
}

//---------------------------------------------------------------------------
// Constructor — empty mesh
//---------------------------------------------------------------------------

TEST(TriangleMeshConstructor, EmptyMeshThrows) {
    const std::string stl = R"(
solid empty
endsolid empty
)";
    std::ofstream f(kTestStlPath);
    ASSERT_TRUE(f) << "failed to create " << kTestStlPath;
    f << stl;
    f.close();

    EXPECT_THROW({ TriangleMesh mesh(kTestStlPath); }, std::invalid_argument);
}

//---------------------------------------------------------------------------
// Constructor — degenerate triangles
//---------------------------------------------------------------------------

TEST(TriangleMeshConstructor, DegenerateDuplicateVerticesThrows) {
    // Triangle with a == b
    const std::string stl = R"(
solid degenerate
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 0 0 0
      vertex 0 1 0
    endloop
  endfacet
endsolid degenerate
)";
    EXPECT_THROW({ make_mesh_from_stl(stl); }, std::invalid_argument);
}

TEST(TriangleMeshConstructor, DegenerateCollinearVerticesThrows) {
    // Three collinear points (zero area)
    const std::string stl = R"(
solid degenerate
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 2 0 0
    endloop
  endfacet
endsolid degenerate
)";
    EXPECT_THROW({ make_mesh_from_stl(stl); }, std::invalid_argument);
}

//---------------------------------------------------------------------------
// Constructor — non-manifold
//---------------------------------------------------------------------------

TEST(TriangleMeshConstructor, NonManifoldEdgeSharedByThreeTrianglesThrows) {
    // Three triangles all sharing the edge (0,0,0)-(1,0,0)
    const std::string stl = R"(
solid nonmanifold
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0.5 1 0
    endloop
  endfacet
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0.5 0.5 0
    endloop
  endfacet
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0.5 -0.5 0
    endloop
  endfacet
endsolid nonmanifold
)";
    EXPECT_THROW({ make_mesh_from_stl(stl); }, std::invalid_argument);
}

//---------------------------------------------------------------------------
// Constructor — valid manifold mesh
//---------------------------------------------------------------------------

TEST(TriangleMeshConstructor, ValidManifoldTwoTrianglesSucceeds) {
    const std::string stl = R"(
solid two_tri
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0 1 0
    endloop
  endfacet
  facet normal 0 0 1
    outer loop
      vertex 1 0 0
      vertex 1 1 0
      vertex 0 1 0
    endloop
  endfacet
endsolid two_tri
)";
    EXPECT_NO_THROW({
        TriangleMesh mesh = make_mesh_from_stl(stl);
        EXPECT_EQ(mesh.GetTriangles().size(), 2u);
    });
}

//---------------------------------------------------------------------------
// GetTriangles
//---------------------------------------------------------------------------

TEST(TriangleMeshGetTriangles, ReturnsParsedTriangles) {
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
    const auto& triangles = mesh.GetTriangles();
    ASSERT_EQ(triangles.size(), 1u);
    expect_point_eq(triangles[0].a, {0., 0., 0.});
    expect_point_eq(triangles[0].b, {1., 0., 0.});
    expect_point_eq(triangles[0].c, {0., 1., 0.});
}

//---------------------------------------------------------------------------
// GetEdgeConnectivity / BuildEdgeConnectivity
//---------------------------------------------------------------------------

TEST(TriangleMeshEdgeConnectivity, SingleTriangleAllEdgesBoundary) {
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
    const auto& conn = mesh.GetEdgeConnectivity();
    EXPECT_EQ(conn.size(), 3u);  // three edges

    Point a{0., 0., 0.}, b{1., 0., 0.}, c{0., 1., 0.};
    Edge e_ab = make_edge(a, b);
    Edge e_bc = make_edge(b, c);
    Edge e_ca = make_edge(c, a);

    auto it_ab = conn.find(e_ab);
    auto it_bc = conn.find(e_bc);
    auto it_ca = conn.find(e_ca);
    ASSERT_NE(it_ab, conn.end());
    ASSERT_NE(it_bc, conn.end());
    ASSERT_NE(it_ca, conn.end());

    // Each edge has one triangle (index 0), second slot is boundary sentinel
    EXPECT_EQ(it_ab->second[0], 0);
    EXPECT_EQ(it_ab->second[1], kBoundaryTriangleIndex);
    EXPECT_EQ(it_bc->second[0], 0);
    EXPECT_EQ(it_bc->second[1], kBoundaryTriangleIndex);
    EXPECT_EQ(it_ca->second[0], 0);
    EXPECT_EQ(it_ca->second[1], kBoundaryTriangleIndex);
}

TEST(TriangleMeshEdgeConnectivity, TwoTrianglesSharedEdgeHasTwoIndices) {
    const std::string stl = R"(
solid two
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0 1 0
    endloop
  endfacet
  facet normal 0 0 1
    outer loop
      vertex 1 0 0
      vertex 1 1 0
      vertex 0 1 0
    endloop
  endfacet
endsolid two
)";
    TriangleMesh mesh = make_mesh_from_stl(stl);
    const auto& conn = mesh.GetEdgeConnectivity();

    // Shared edge (0,1,0)-(1,0,0) in canonical form is (0,1,0), (1,0,0)
    Point p01{0., 1., 0.}, p10{1., 0., 0.};
    Edge shared = make_edge(p01, p10);
    auto it = conn.find(shared);
    ASSERT_NE(it, conn.end());
    const auto& adj = it->second;
    EXPECT_NE(adj[0], kBoundaryTriangleIndex);
    EXPECT_NE(adj[1], kBoundaryTriangleIndex);
    // One index is 0, the other is 1
    EXPECT_TRUE((adj[0] == 0 && adj[1] == 1) || (adj[0] == 1 && adj[1] == 0));
}

TEST(TriangleMeshEdgeConnectivity, TwoTrianglesBoundaryEdgesHaveSentinel) {
    const std::string stl = R"(
solid two
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0 1 0
    endloop
  endfacet
  facet normal 0 0 1
    outer loop
      vertex 1 0 0
      vertex 1 1 0
      vertex 0 1 0
    endloop
  endfacet
endsolid two
)";
    TriangleMesh mesh = make_mesh_from_stl(stl);
    const auto& conn = mesh.GetEdgeConnectivity();

    // Boundary edge only on first triangle: (0,0,0)-(1,0,0)
    Edge e_00_10 = make_edge({0., 0., 0.}, {1., 0., 0.});
    auto it = conn.find(e_00_10);
    ASSERT_NE(it, conn.end());
    EXPECT_EQ(it->second[0], 0);
    EXPECT_EQ(it->second[1], kBoundaryTriangleIndex);
}
