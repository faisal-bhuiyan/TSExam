#pragma once

#include <istream>
#include <ostream>
#include <span>
#include <string_view>
#include <vector>

#include "geometry.hpp"

namespace tsexam::problem1 {

std::vector<Triangle> parse_ascii_stl(std::istream&);

void write_triangle_in_ascii_stl(std::ostream&, const Triangle&);

void write_ascii_stl(std::ostream&, std::string_view, std::span<const Triangle>);

}  // namespace tsexam::problem1
