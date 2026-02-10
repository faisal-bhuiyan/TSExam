#include "polyline.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace tsexam::problem2 {

Polyline::Polyline(
    PolylineRepresentation representation, const std::vector<size_t>& data,
    std::vector<Point> vertices
)
    : vertices_(std::move(vertices)) {
    //----------------------------------------------
    // Checks
    //----------------------------------------------
    // Segment buffer is empty -> throw
    if (data.empty()) {
        throw std::invalid_argument("segments buffer cannot be empty");
    }

    if (representation == PolylineRepresentation::kVerboseSegments) {
        // Number of entries in segment data is NOT even -> throw
        if (data.size() % 2 != 0) {
            throw std::invalid_argument("segments buffer must contain an even number of entries");
        }

        /**
         * NOTE: This validation is not optimal for performance, but it establishes the assumptions
         * made into the class invariants i.e. single-connected polyline/polygon and degree is 2 (1
         * for endpoints). I am prioritizing robustness over performance here. The hot path in
         * GetCompressedVertexOrdering() assumes valid input.
         */

        // Find the vertex with largest index in the segments buffer to determine vertex count
        const size_t num_segments{data.size() / 2};
        size_t max_vertex{0};
        for (auto vertex : data) {
            max_vertex = std::max(max_vertex, vertex);
        }

        // Count degree of each vertex i.e. number of segments it participates in
        std::vector<size_t> degree_of_vertices(max_vertex + 1, 0);
        for (size_t i = 0; i < num_segments; ++i) {
            // Each segment connects two vertices -> increment degree counts
            ++degree_of_vertices[data[2 * i]];
            ++degree_of_vertices[data[2 * i + 1]];
        }

        // Validate single-connected polyline/polygon assumptions:
        // - every vertex has degree 1 or 2
        // - exactly 0 degree-1 vertices (polygon) or exactly 2 vertices (open polyline)
        size_t degree_1_count{0};
        for (size_t v = 0; v <= max_vertex; ++v) {
            // Ignore unconnected vertices (degree 0)
            if (degree_of_vertices[v] == 0) {
                continue;
            }

            // Count degree-1 endpoints
            if (degree_of_vertices[v] == 1) {
                ++degree_1_count;
            }

            // Validate degree <= 2 for single-connected polyline/polygon
            if (degree_of_vertices[v] > 2) {
                throw std::invalid_argument(
                    "vertex " + std::to_string(v) + " has degree " +
                    std::to_string(degree_of_vertices[v]) +
                    "; expected at most 2 for a single connected polyline/polygon"
                );
            }
        }

        // We have a problem if number of degree-1 endpoints is NOT 0 or 2 -> throw
        if (degree_1_count != 0 && degree_1_count != 2) {
            throw std::invalid_argument(
                "expected 0 or 2 degree-1 endpoints, found " + std::to_string(degree_1_count)
            );
        }

        // Now that we have validated the input, we can build the compressed vertex ordering
        // and use it for storing the segments with optimal memory footprint
        this->compressed_segments_ = GetCompressedVertexOrdering(data, max_vertex + 1);

    } else if (representation == PolylineRepresentation::kCompressedVertexOrdering) {
        // Data is already in compressed vertex ordering form -> store directly
        this->compressed_segments_ = data;
    } else {
        // Invalid representation value -> throw
        throw std::invalid_argument("Invalid PolylineRepresentation value: not supported");
    }

    // Determine polyline type based on compressed vertex ordering
    this->type_ = IsPolygon() ? PolylineType::kClosed : PolylineType::kOpen;
}

std::vector<size_t> Polyline::GetCompressedVertexOrdering(
    std::span<const size_t> segments, size_t num_vertices
) {
    //----------------------------------------------
    // Build vertex connectivity
    //----------------------------------------------

    const size_t num_segments{segments.size() / 2};

    // Each vertex connects to up to two others; -1 = unconnected
    std::vector<std::pair<int, int>> vertex_connectivity(num_vertices, {-1, -1});

    // Assign a neighbor to the first available slot (-1) of a vertex
    auto assign_neighbor = [&vertex_connectivity](int vertex, int neighbor) {
        if (vertex_connectivity[vertex].first == -1) {
            vertex_connectivity[vertex].first = neighbor;
            return;
        }
        vertex_connectivity[vertex].second = neighbor;
    };

    // Each segment connects two vertices -> build connectivity pairs
    for (size_t index = 0; index < num_segments; ++index) {
        const int vertex_1 = static_cast<int>(segments[2 * index]);
        const int vertex_2 = static_cast<int>(segments[2 * index + 1]);
        assign_neighbor(vertex_1, vertex_2);
        assign_neighbor(vertex_2, vertex_1);
    }

    //----------------------------------------------
    // Determine polyline type
    //----------------------------------------------

    // Find endpoints (degree-1 vertices: only .first is set AND .second is -1)
    std::vector<size_t> endpoints;
    for (size_t vertex = 0; vertex < num_vertices; ++vertex) {
        if (vertex_connectivity[vertex].first != -1 && vertex_connectivity[vertex].second == -1) {
            endpoints.push_back(vertex);
        }
    }

    // Two scenarios possible:
    // Polyline: 2 endpoints (2 vertices w/ degree 1, rest degree 2)
    // Polygon: no endpoints (all vertices w/ degree 2)
    const bool is_closed = endpoints.empty();
    size_t starting_vertex{0};
    if (is_closed) {
        // Polygon -> start at the smallest participating vertex
        for (size_t vertex = 0; vertex < num_vertices; ++vertex) {
            if (vertex_connectivity[vertex].first != -1) {
                starting_vertex = vertex;
                break;
            }
        }
    } else {
        // Open polyline -> start at the smaller endpoint of the two
        starting_vertex = std::min(endpoints[0], endpoints[1]);
    }

    //----------------------------------------------
    // Build compressed vertex ordering
    //----------------------------------------------

    // Walk the chain: at each step, advance to the next neighbor that is not the previous vertex
    std::vector<size_t> compressed_ordering;
    compressed_ordering.reserve(num_vertices + (is_closed ? 1 : 0));
    compressed_ordering.push_back(starting_vertex);

    // Lambda: given a vertex, return the neighbor that is not `already_visited`
    // Returns -1 if no such neighbor exists (reached the end of open polyline)
    auto next_neighbor = [&vertex_connectivity](size_t vertex, size_t already_visited) -> int {
        auto [v1, v2] = vertex_connectivity[vertex];
        if (v1 != -1 && static_cast<size_t>(v1) != already_visited) {
            return v1;
        }
        if (v2 != -1 && static_cast<size_t>(v2) != already_visited) {
            return v2;
        }
        // No unvisited neighbors -> end of open polyline
        return -1;
    };

    // First step: just take the first neighbor (determinism for polylines is already guaranteed by
    // starting at the smaller endpoint for open polylines)
    size_t previous_vertex{starting_vertex};
    size_t current_vertex{static_cast<size_t>(vertex_connectivity[starting_vertex].first)};
    compressed_ordering.push_back(current_vertex);

    // Subsequent steps: walk the chain until we reach the end of an open polyline or loop back to
    // the start of a polygon
    while (true) {
        int next_vertex = next_neighbor(current_vertex, previous_vertex);

        // Break condition for polyline: reached the other endpoint
        if (next_vertex == -1) {
            break;
        }

        compressed_ordering.push_back(static_cast<size_t>(next_vertex));

        // Break condition for polygon: completed the loop back to the starting vertex
        if (is_closed && static_cast<size_t>(next_vertex) == starting_vertex) {
            break;
        }

        // Advance the walk: update previous and current vertices
        previous_vertex = current_vertex;
        current_vertex = static_cast<size_t>(next_vertex);
    }

    return compressed_ordering;
}

bool Polyline::IsPolygon() const {
    // A polygon's compressed ordering starts and ends with the same vertex
    return compressed_segments_.size() >= 2 &&
           compressed_segments_.front() == compressed_segments_.back();
}

}  // namespace tsexam::problem2
