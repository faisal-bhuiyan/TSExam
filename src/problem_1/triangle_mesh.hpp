#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "geometry.hpp"

namespace tsexam::problem1 {

/// Triangle mesh loaded from an STL file
class TriangleMesh {
public:
    TriangleMesh() = default;

    /// Construct a mesh by parsing an ASCII STL file at provided path
    explicit TriangleMesh(const std::string& path);

    /// Build the edge-to-triangle connectivity map
    void BuildEdgeConnectivity();

    const std::vector<Triangle>& GetTriangles() const { return triangles_; }

    const std::unordered_map<Edge, std::vector<std::size_t>, EdgeHash, EdgeEquality>&
    GetEdgeConnectivity() const {
        return edge_connectivity_;
    }

private:
    /// List of triangles in the mesh
    std::vector<Triangle> triangles_;

    /// Maps each (canonical) edge to the indices of triangles that share it
    std::unordered_map<Edge, std::vector<std::size_t>, EdgeHash, EdgeEquality> edge_connectivity_;
};

}  // namespace tsexam::problem1
