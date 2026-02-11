#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <utility>

namespace tsexam::problem1 {

//----------------------------------------------
// Point
//----------------------------------------------

using Point = std::array<double, 3>;

struct PointHash {
    std::size_t operator()(const Point& p) const noexcept {
        std::size_t h1 = std::hash<double>{}(p[0]);  // hash x coordinate
        std::size_t h2 = std::hash<double>{}(p[1]);  // hash y coordinate
        std::size_t h3 = std::hash<double>{}(p[2]);  // hash z coordinate

        // Combine hashes using boost-style technique
        std::size_t hash = h1;
        hash ^= h2 + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
        hash ^= h3 + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
        return hash;
    }
};

struct PointEquality {
    bool operator()(const Point& a, const Point& b) const noexcept {
        return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
    }
};

//----------------------------------------------
// Edge
//----------------------------------------------

/// An edge in canonical form -> first <= second
using Edge = std::pair<Point, Point>;

/// Build a canonical edge from two points
inline Edge make_edge(const Point& p, const Point& q) {
    return (p < q) ? Edge{p, q} : Edge{q, p};
}

struct EdgeHash {
    std::size_t operator()(const Edge& e) const noexcept {
        PointHash point_hash;
        std::size_t h1 = point_hash(e.first);
        std::size_t h2 = point_hash(e.second);

        // Combine hashes using boost-style technique
        std::size_t hash = h1;
        hash ^= h2 + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
        return hash;
    }
};

struct EdgeEquality {
    bool operator()(const Edge& e1, const Edge& e2) const noexcept {
        PointEquality eq;
        return eq(e1.first, e2.first) && eq(e1.second, e2.second);
    }
};

//----------------------------------------------
// Triangle
//----------------------------------------------

/**
 * @brief Triangle in 3D for storing STL mesh data
 */
struct Triangle {
    Point a;  //< first vertex
    Point b;  //< second vertex
    Point c;  //< third vertex
};

}  // namespace tsexam::problem1
