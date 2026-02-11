#pragma once

#include "geometry.hpp"
#include "triangle_mesh.hpp"

namespace tsexam::problem1 {

/// Flip a triangle by swapping the second and third vertices
void flip_triangle(Triangle&);

/// Check if a triangle has a directed edge from one vertex to another
bool has_directed_edge(const Triangle&, const Point&, const Point&);

/// Check if two triangles have consistent orientations for a given edge
bool are_orientations_consistent(const Triangle&, const Triangle&, const Edge&);

/**
 * @brief Reorient the triangles in a mesh that have inconsistent orientations
 * @param mesh The mesh to reorient
 * @param seed The index of the seed triangle
 */
std::vector<Triangle> reorient_inconsistent_triangles(TriangleMesh&, std::size_t seed);

void export_inconsistent_triangles(TriangleMesh&, std::size_t seed, std::ostream&);

}  // namespace tsexam::problem1
