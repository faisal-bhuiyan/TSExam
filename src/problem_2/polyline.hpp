#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace tsexam::problem2 {

using Point = std::array<double, 3>;
using VertexIndex = int32_t;

enum class PolylineType {
    kOpen = 0,    ///< Open polyline with distinct start and end vertices
    kClosed = 1,  ///< Closed polyline where start and end vertices are the same
};

enum class PolylineRepresentation {
    kVerboseSegments = 0,           ///< A flat list of vertex index pairs representing segments
    kCompressedVertexOrdering = 1,  ///< A sequence of vertex indices in traversal order
};

class Polyline {
public:
    Polyline(PolylineRepresentation, const std::vector<VertexIndex>&, std::vector<Point> = {});

    const std::vector<Point>& GetVertices() const { return vertices_; }
    const std::vector<VertexIndex>& GetCompressedSegments() const { return compressed_segments_; }
    PolylineType GetType() const { return type_; }

    /// Convert verbose raw segments representation -> compressed vertex ordering
    static std::vector<VertexIndex> GetCompressedVertexOrdering(
        std::span<const VertexIndex>, size_t num_vertices
    );

    /// Return the type of the polyline (open vs closed) based on its compressed vertex ordering
    bool IsPolygon() const;

private:
    /// List of vertices in the polyline
    std::vector<Point> vertices_;

    /// List of compressed segments in the polyline
    std::vector<VertexIndex> compressed_segments_;

    /// Type of the polyline (open vs closed)
    PolylineType type_;
};

}  // namespace tsexam::problem2
