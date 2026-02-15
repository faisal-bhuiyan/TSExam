# Problem 2 — Polyline

## Overview

This problem involves storing the segments of a polyline in a more compact  and detecting if the line is open/closed. This is the first problem I solved for this exam and it is probably the easiest problem of the bunch. Solving graph traversal problems was something new for me, however it
was a fun problem to work through.

## Approach

Prompt: High-level plan, assumptions, algorithmic choices, and complexity trade-offs.

- **High-level plan:**
    - Perform the conversion from verbose line segment in the c-tor of the Polyline class and use the compressed representation to store the
    polyline. This way, we could just do the conversion once and use the class attribute when we need the polyline.
    - Algorithm choice was breadth first search to establish the edge connectivity of the vertices. It is a linear complexity algorithm O(n).
    - See the handwritten notes for further details on the data structure and algorithm design decisions (attached as a pdf).

- **Input formats:** Two representations: verbose segments (flat list of 2N vertex index pairs) and compressed vertex ordering (traversal order). Constructor validates then either converts via `GetCompressedVertexOrdering` or stores compressed data directly.

- **Assumptions:**
    - Input describes a single-connected polyline (open chain) or polygon (closed chain).
    - Every vertex has at most 2 segments attached: exactly 0 degree-1 vertices for polygons and exactly 2 degree-1 vertices for polylines.
    - Non-contiguous vertex indices (gaps) are allowed, degree computed only over participating vertices.

- **Algorithmic choices:**
    - Build adjacency from segments (each vertex stores up to two neighbors)
    - For open polylines start walk at the smaller endpoint (determinism).
    - For closed start at smallest participating vertex; walk by "next neighbor not equal to previous" until endpoint or back to start.

- **Complexity / trade-offs:** O(segments) to build connectivity and validate; O(vertices) for degree array and walk. Validation is not optimized for huge inputs but prioritizes robustness and clear invariants for the hot path (see comment in `polyline.cpp` lines 29–34).

## Work / Derivation

Prompt: Key derivations, proofs, or notes showing how you arrived at formulas or algorithms.

- **Degree invariants:** For a single-connected polyline/polygon, every vertex has degree 1 (endpoints) or 2 (internal). Valid inputs have exactly 0 or 2 degree-1 vertices; any other count or any degree > 2 throws.

- **GetCompressedVertexOrdering:** (1) Build `vertex_connectivity[vertex]` as two neighbor slots from segment list. (2) Endpoints = vertices with exactly one neighbor. (3) If 2 endpoints → open; start at `min(endpoint0, endpoint1)`. If 0 endpoints → closed; start at smallest participating vertex. (4) Walk: push current, set next = neighbor ≠ previous; stop when next is unconnected (open) or next == starting_vertex (closed). For closed, append starting vertex again so compressed form is `[v0, v1, …, v0]`.

- **IsPolygon:** Implemented as `compressed_segments_.size() >= 2 && front() == back()` (`polyline.cpp` lines 225–229). Type is set once after construction from this check.

## Tests / Results

Prompt: Short description of tests you ran, inputs used, and important outputs or timings.

- **Part A — Determinism:** 7 tests (e.g. CanonicalOrder, PermutedSegmentOrder, FlippedVerticesWithinSegments, ExamFigure3Example). Same logical chain (e.g. 0—1—2—3) with permuted/flipped segments yields identical compressed ordering `[0,1,2,3]`.

- **Part A — Edge cases:** Single segment, two segments, longer chain, non-contiguous indices; all assert correct compressed ordering and start-at-smaller-endpoint.

- **Part A — Polygon:** Closed triangle/quad from segments (permuted and flipped); ordering starts and ends at same vertex, starts at smallest (0).

- **Part B — IsPolygon / GetType:** Open vs closed from both segment and compressed input; roundtrip (build from segments, get compressed, build from compressed, same type and ordering).

- **Input validation:** Empty data, odd segment count, vertex degree > 2, four endpoints (disconnected), duplicate segment, self-loop — all expect `std::invalid_argument`.

- **Other:** Vertices constructor (optional coordinates stored; behavior unchanged); sparse vertex indices (gaps in index space); static `GetCompressedVertexOrdering` with span; performance tests (very long open polyline, worst-case permutation).

- **How to run:** `./build/problem2_tests` (or `build\Debug\problem2_tests.exe` on Windows). No special inputs; all tests use in-memory vectors.

## Answer

Prompt: Succinct final answer or summary of deliverables (what files to run, how to run them).

- **Deliverables:** `src/problem_2/polyline.hpp` (API: `Polyline`, `PolylineRepresentation`, `PolylineType`, `GetCompressedVertexOrdering`), `src/problem_2/polyline.cpp` (implementation), `tests/problem_2/test_polyline.cpp` (Google Test suite).

- **Build:** From repo root: `cmake -B build -S .` then `cmake --build build`.

- **Run tests:** `ctest --test-dir build -R problem2 --output-on-failure` or run the `problem2_tests` executable directly from the build directory.

## Rubric

Prompt: A short self-assessment (optional) indicating what parts you believe deserve full credit and what is incomplete.

Partial credit will be awarded for correct approaches and clear reasoning even if a complete implementation is not finished. Explicitly document any limitations and the most important next steps.

- **Self-assessment:** Implementation is complete: both input formats (verbose segments and compressed vertex ordering), full validation (degree and endpoint invariants), deterministic compressed ordering (open: start at smaller endpoint; closed: start at smallest participating vertex), open/closed classification via `IsPolygon()` and `GetType()`, and broad test coverage (determinism, edge cases, polygons, roundtrip, validation, sparse indices, vertices constructor, performance).

- **Limitations / next steps:** (1) Validation does two passes over segment data and allocates a degree array of size max vertex index — for very large sparse inputs this could be optimized. (2) Optional next steps: add a method to return segment count or edge list; consider supporting multiple disconnected components if the problem is extended.
