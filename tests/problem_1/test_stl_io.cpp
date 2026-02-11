#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "problem_1/stl_io.hpp"

using tsexam::problem1::parse_ascii_stl;
using tsexam::problem1::Point;
using tsexam::problem1::Triangle;
using tsexam::problem1::write_ascii_stl;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Parse an ASCII STL string and return the resulting triangles
static std::vector<Triangle> parse(const std::string& stl_text) {
    std::istringstream stream(stl_text);
    return parse_ascii_stl(stream);
}

/// Check that two points are equal within a tolerance
static void expect_point_eq(const Point& actual, const Point& expected) {
    EXPECT_DOUBLE_EQ(actual[0], expected[0]);
    EXPECT_DOUBLE_EQ(actual[1], expected[1]);
    EXPECT_DOUBLE_EQ(actual[2], expected[2]);
}

// ---------------------------------------------------------------------------
// Empty / degenerate input
// ---------------------------------------------------------------------------

TEST(ParseAsciiStl, EmptyInputReturnsNoTriangles) {
    auto triangles = parse("");
    EXPECT_TRUE(triangles.empty());
}

TEST(ParseAsciiStl, NoVertexKeywordsReturnsNoTriangles) {
    auto triangles = parse("solid cube\nendsolid cube\n");
    EXPECT_TRUE(triangles.empty());
}

// ---------------------------------------------------------------------------
// Single triangle
// ---------------------------------------------------------------------------

TEST(ParseAsciiStl, SingleTriangle) {
    const std::string stl = R"(
solid single
  facet normal 0 0 1
    outer loop
      vertex 0.0 0.0 0.0
      vertex 1.0 0.0 0.0
      vertex 0.0 1.0 0.0
    endloop
  endfacet
endsolid single
)";
    auto triangles = parse(stl);
    ASSERT_EQ(triangles.size(), 1u);
    expect_point_eq(triangles[0].a, {0., 0., 0.});
    expect_point_eq(triangles[0].b, {1., 0., 0.});
    expect_point_eq(triangles[0].c, {0., 1., 0.});
}

// ---------------------------------------------------------------------------
// Multiple triangles
// ---------------------------------------------------------------------------

TEST(ParseAsciiStl, TwoTriangles) {
    const std::string stl = R"(
solid two
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0 1 0
    endloop
  endfacet
  facet normal 0 0 -1
    outer loop
      vertex 1 0 0
      vertex 1 1 0
      vertex 0 1 0
    endloop
  endfacet
endsolid two
)";
    auto triangles = parse(stl);
    ASSERT_EQ(triangles.size(), 2u);

    expect_point_eq(triangles[0].a, {0, 0, 0});
    expect_point_eq(triangles[0].b, {1, 0, 0});
    expect_point_eq(triangles[0].c, {0, 1, 0});

    expect_point_eq(triangles[1].a, {1, 0, 0});
    expect_point_eq(triangles[1].b, {1, 1, 0});
    expect_point_eq(triangles[1].c, {0, 1, 0});
}

// ---------------------------------------------------------------------------
// Negative coordinates
// ---------------------------------------------------------------------------

TEST(ParseAsciiStl, NegativeCoordinates) {
    const std::string stl = R"(
solid neg
  facet normal 0 0 1
    outer loop
      vertex -1.5 -2.5 -3.5
      vertex  4.0  5.0  6.0
      vertex -7.0  8.0 -9.0
    endloop
  endfacet
endsolid neg
)";
    auto triangles = parse(stl);
    ASSERT_EQ(triangles.size(), 1u);
    expect_point_eq(triangles[0].a, {-1.5, -2.5, -3.5});
    expect_point_eq(triangles[0].b, {4., 5., 6.});
    expect_point_eq(triangles[0].c, {-7., 8., -9.});
}

// ---------------------------------------------------------------------------
// Scientific notation
// ---------------------------------------------------------------------------

TEST(ParseAsciiStl, ScientificNotation) {
    const std::string stl = R"(
solid sci
  facet normal 0 0 1
    outer loop
      vertex 1.5e2 -3.0e-1 0.0e0
      vertex 1e3 2e-4 3e+1
      vertex -1.23e+2 4.56e-3 7.89e0
    endloop
  endfacet
endsolid sci
)";
    auto triangles = parse(stl);
    ASSERT_EQ(triangles.size(), 1u);
    expect_point_eq(triangles[0].a, {150., -0.3, 0.});
    expect_point_eq(triangles[0].b, {1000., 0.0002, 30.});
    expect_point_eq(triangles[0].c, {-123., 0.00456, 7.89});
}

// ---------------------------------------------------------------------------
// Whitespace variations
// ---------------------------------------------------------------------------

TEST(ParseAsciiStl, ExtraWhitespaceAndTabs) {
    // Tabs, extra spaces, blank lines — parser should handle all whitespace uniformly
    const std::string stl =
        "solid ws\n"
        "\n"
        "  facet normal 0 0 1\n"
        "    outer loop\n"
        "\t\tvertex\t\t1.0   2.0   3.0\n"
        "      vertex   4.0\t5.0\t6.0\n"
        "\n"
        "      vertex 7.0 8.0 9.0\n"
        "    endloop\n"
        "  endfacet\n"
        "endsolid ws\n";

    auto triangles = parse(stl);
    ASSERT_EQ(triangles.size(), 1u);
    expect_point_eq(triangles[0].a, {1., 2., 3.});
    expect_point_eq(triangles[0].b, {4., 5., 6.});
    expect_point_eq(triangles[0].c, {7., 8., 9.});
}

// ---------------------------------------------------------------------------
// Minimal (no header/footer keywords)
// ---------------------------------------------------------------------------

TEST(ParseAsciiStl, MinimalVertexOnlyInput) {
    // The parser only cares about "vertex" tokens; everything else is ignored
    const std::string stl =
        "vertex 0 0 0\n"
        "vertex 1 0 0\n"
        "vertex 0 1 0\n";

    auto triangles = parse(stl);
    ASSERT_EQ(triangles.size(), 1u);
    expect_point_eq(triangles[0].a, {0, 0, 0});
    expect_point_eq(triangles[0].b, {1, 0, 0});
    expect_point_eq(triangles[0].c, {0, 1, 0});
}

// ---------------------------------------------------------------------------
// Incomplete triangle (only 2 vertices)
// ---------------------------------------------------------------------------

TEST(ParseAsciiStl, IncompleteTriangleIsDropped) {
    // Only two vertices — not enough to form a triangle
    const std::string stl = R"(
solid incomplete
  facet normal 0 0 1
    outer loop
      vertex 1 2 3
      vertex 4 5 6
    endloop
  endfacet
endsolid incomplete
)";
    auto triangles = parse(stl);
    EXPECT_TRUE(triangles.empty());
}

// ---------------------------------------------------------------------------
// Mixed: one complete + one incomplete triangle
// ---------------------------------------------------------------------------

TEST(ParseAsciiStl, CompleteTriangleFollowedByIncomplete) {
    const std::string stl = R"(
solid mixed
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 1 0 0
      vertex 0 1 0
    endloop
  endfacet
  facet normal 0 0 1
    outer loop
      vertex 2 2 2
      vertex 3 3 3
    endloop
  endfacet
endsolid mixed
)";
    auto triangles = parse(stl);
    ASSERT_EQ(triangles.size(), 1u);
    expect_point_eq(triangles[0].a, {0, 0, 0});
}

// ---------------------------------------------------------------------------
// Large coordinate values
// ---------------------------------------------------------------------------

TEST(ParseAsciiStl, LargeCoordinateValues) {
    const std::string stl = R"(
solid large
  facet normal 0 0 1
    outer loop
      vertex 999999.999 -999999.999 0.000001
      vertex 123456789.0 0.0 0.0
      vertex 0.0 987654321.0 0.0
    endloop
  endfacet
endsolid large
)";
    auto triangles = parse(stl);
    ASSERT_EQ(triangles.size(), 1u);
    expect_point_eq(triangles[0].a, {999999.999, -999999.999, 0.000001});
    expect_point_eq(triangles[0].b, {123456789., 0., 0.});
    expect_point_eq(triangles[0].c, {0., 987654321., 0.});
}

// ---------------------------------------------------------------------------
// Integer coordinates (no decimal point)
// ---------------------------------------------------------------------------

TEST(ParseAsciiStl, IntegerCoordinates) {
    const std::string stl = R"(
solid ints
  facet normal 0 0 1
    outer loop
      vertex 1 2 3
      vertex 4 5 6
      vertex 7 8 9
    endloop
  endfacet
endsolid ints
)";
    auto triangles = parse(stl);
    ASSERT_EQ(triangles.size(), 1u);
    expect_point_eq(triangles[0].a, {1, 2, 3});
    expect_point_eq(triangles[0].b, {4, 5, 6});
    expect_point_eq(triangles[0].c, {7, 8, 9});
}

// ---------------------------------------------------------------------------
// Write tests
// ---------------------------------------------------------------------------

TEST(WriteAsciiStl, SingleTriangleRoundTrip) {
    Triangle t{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};

    std::ostringstream out;
    write_ascii_stl(out, "single", std::span<const Triangle>{&t, 1});

    auto parsed = parse(out.str());
    ASSERT_EQ(parsed.size(), 1u);
    expect_point_eq(parsed[0].a, t.a);
    expect_point_eq(parsed[0].b, t.b);
    expect_point_eq(parsed[0].c, t.c);
}

TEST(WriteAsciiStl, MultipleTrianglesRoundTrip) {
    std::vector<Triangle> tris{
        {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}},
        {{1, 0, 0}, {1, 1, 0}, {0, 1, 0}},
    };

    std::ostringstream out;
    write_ascii_stl(out, "two", tris);

    auto parsed = parse(out.str());
    ASSERT_EQ(parsed.size(), tris.size());
}

TEST(WriteAsciiStl, DegenerateTriangleZeroArea) {
    Triangle t{{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};

    std::ostringstream out;
    write_ascii_stl(out, "degenerate", std::span<const Triangle>{&t, 1});

    auto parsed = parse(out.str());
    ASSERT_EQ(parsed.size(), 1u);
}

TEST(WriteAsciiStl, NegativeAndLargeCoordinates) {
    Triangle t{{-1e6, 2.5, -3.5}, {4., -5e5, 6.}, {7., 8., -9e4}};

    std::ostringstream out;
    write_ascii_stl(out, "coords", std::span<const Triangle>{&t, 1});

    auto parsed = parse(out.str());
    ASSERT_EQ(parsed.size(), 1u);
}

TEST(WriteAsciiStl, EmptyTriangleList) {
    std::ostringstream out;
    write_ascii_stl(out, "empty", {});

    EXPECT_EQ(out.str(), "solid empty\nendsolid empty\n");
}

TEST(WriteAsciiStl, DeterministicOutput) {
    Triangle t{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};

    std::ostringstream out1, out2;
    write_ascii_stl(out1, "det", std::span<const Triangle>{&t, 1});
    write_ascii_stl(out2, "det", std::span<const Triangle>{&t, 1});

    EXPECT_EQ(out1.str(), out2.str());
}
