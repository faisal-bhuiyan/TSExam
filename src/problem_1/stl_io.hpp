#pragma once

#include <istream>
#include <ostream>
#include <span>
#include <string_view>
#include <vector>

#include "geometry.hpp"

namespace tsexam::problem1 {

/**
 * @brief Parses an ASCII STL stream and extracts triangle geometry
 *
 * The parser scans the input stream for `vertex` tokens and groups every three vertices into a
 * triangle. All other tokens and keywords are ignored.
 *
 * @param input Input stream containing ASCII STL data
 * @return List of parsed triangles
 */
std::vector<Triangle> parse_ascii_stl(std::istream&);

/**
 * @brief Writes a single triangle to an output stream in ASCII STL format
 *
 * The triangle is emitted as a `facet` with an unnormalized normal computed from the vertex
 * ordering.
 *
 * @param out Output stream to write to
 * @param triangle Triangle geometry to emit
 */
void write_triangle_in_ascii_stl(std::ostream&, const Triangle&);

/**
 * @brief Writes a collection of triangles to an output stream in ASCII STL format
 *
 * The output includes a `solid` header and `endsolid` footer using the provided solid name. Each
 * triangle is written as an individual facet.
 *
 * @param out Output stream to write to
 * @param name Name of the STL solid
 * @param triangles Collection of triangles to emit
 */
void write_ascii_stl(std::ostream&, std::string_view name, std::span<const Triangle>);

}  // namespace tsexam::problem1
