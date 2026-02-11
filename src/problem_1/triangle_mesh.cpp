#include "triangle_mesh.hpp"

#include <fstream>

#include "stl_io.hpp"

namespace tsexam::problem1 {

TriangleMesh::TriangleMesh(const std::string& path) {
    std::ifstream file(path);
    if (file) {
        triangles_ = parse_ascii_stl(file);
    }

    BuildEdgeConnectivity();
}

void TriangleMesh::BuildEdgeConnectivity() {
    edge_connectivity_.clear();

    // For each triangle, add its edges to the edge-to-triangle connectivity map
    for (std::size_t i = 0; i < triangles_.size(); ++i) {
        const auto& triangle = triangles_[i];

        // Each triangle contributes three edges in canonical form
        edge_connectivity_[make_edge(triangle.a, triangle.b)].push_back(i);
        edge_connectivity_[make_edge(triangle.b, triangle.c)].push_back(i);
        edge_connectivity_[make_edge(triangle.c, triangle.a)].push_back(i);
    }
}

}  // namespace tsexam::problem1
