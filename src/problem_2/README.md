# Problem 2 — Polyline

## Overall thoughts

This problem involves storing the segments of a polyline in a more compact form and detecting whether the line is open or closed. This was the first problem I solved for this exam and is probably the easiest problem of the bunch. Solving graph traversal problems was something new for me; however, it was a fun problem to work through and set me up nicely to solve problem 1.

While this problem could reasonably be solved in a couple hours, I spent 4-5 hours on it to showcase my production code development style and workflow. I am a big believer in well-designed architecture, clean, readable, and maintainable code, with plenty of tests to provide coverage and act as code-based user documentation. I also took the time to add doxygen-style comments to the source (header) files as well as detailed comments in the implementation files to communicate intent and provide guidance for understanding more complex parts of the algorithm.

## Approach

*Prompt: High-level plan, assumptions, algorithmic choices, and complexity trade-offs.*

- **High-level plan:**
  The design strays slightly from the instructions in that I did not implement standalone functions to perform the conversions. Instead, I designed a `Polyline` class that establishes the invariants and assumptions for the data, performs the compression once in the constructor, and stores the result as a class attribute. This approach keeps the representation consistent and avoids repeated work. Several helper functions and enums were added to provide a clean and complete deliverable for the problem.

    - Perform the conversion from verbose line segments in the constructor of the `Polyline` class and store the polyline internally using the compressed representation. This way, the conversion is done once and the compressed form can be reused whenever the polyline is needed.
    - The algorithm builds edge connectivity from the segment list in a single pass, then performs a single walk along the chain to produce the compressed ordering. This is a linear-time algorithm with complexity $O(n)$.
    - See the handwritten notes for further details on the data structures and algorithm design decisions (attached as a PDF).

- **Input formats:**
  Two representations of polylines are supported by the `Polyline` class:
    - Verbose segments (a flat list of 2N vertex index pairs).
    - Compressed vertex ordering (explicit traversal order).
  The constructor validates the input and either converts via `GetCompressedVertexOrdering` or stores the compressed data directly.

- **Assumptions:**
  The assumptions are provided in the exam instructions; I am not assuming anything beyond those:
    - The input describes a single connected polyline (open chain) or polygon (closed chain).
    - Every vertex has at most two segments attached: exactly zero degree-1 vertices for polygons and exactly two degree-1 vertices for open polylines.
    - Non-contiguous vertex indices (gaps) are allowed; degree is computed only over participating vertices.

- **Algorithmic choices:**
    - Build adjacency/connectivity from segments, where each vertex stores up to two neighbors.
    - For open polylines, start the walk at the smaller endpoint to ensure determinism.
    - For closed polylines, start at the smallest participating vertex and walk by selecting the neighbor that is not equal to the previous vertex until returning to the start.

- **Complexity / trade-offs:**
    - $O(segments)$ to build connectivity and validate input.
    - $O(vertices)$ for the degree array and traversal.
    - Validation is not optimized for large inputs but prioritizes robustness and clear invariants for the hot path.


## Work / Derivation

*Prompt: Key derivations, proofs, or notes showing how you arrived at formulas or algorithms.*

More detailed reasoning, intermediate sketches, and design exploration for this problem are provided in the accompanying handwritten notes. What follows captures the key derivation steps and invariants that drive the implementation.

- **Degree invariants:**
  For a single connected polyline or polygon, every vertex has degree 1 (endpoints) or 2 (internal). Valid inputs therefore have exactly 0 or 2 degree-1 vertices. Any other count, or any vertex with degree greater than 2, is considered invalid and results in an exception. These invariants are the foundation for both validation and traversal.

- **GetCompressedVertexOrdering():**
  This routine explicitly solves part (A) of the problem by converting verbose segment input into a deterministic traversal order.
    - Build `vertex_connectivity[vertex]` from the verbose segment list, where each vertex stores up to two neighbor slots.
    - Identify endpoints as vertices with exactly one neighbor.
    - If there are two endpoints, the polyline is open and traversal starts at `min(endpoint0, endpoint1)`. If there are zero endpoints, the polyline is closed and traversal starts at the smallest participating vertex.
    - Walk the polyline by pushing the current vertex and selecting the neighbor that is not equal to the previous vertex. Traversal stops when the next vertex is unconnected (open case) or when the next vertex equals the starting vertex (closed case). For closed polylines, the starting vertex is appended again so the compressed form is `[v0, v1, …, v0]`.

- **IsPolygon():**
  This routine explicitly solves part (B) of the problem.
    - Performed after the compressed ordering is constructed, it reduces to a simple check that the compressed ordering's first and last vertices are the same.
    - The polyline type is determined once during construction and cached (as a class attribute) for later use.

## Tests / Results

*Prompt: Short description of tests you ran, inputs used, and important outputs or timings.*

- **Determinism (Part A):**
  - Demonstrates that the compressed vertex ordering is invariant under permutation of the input segment list.
  - Demonstrates that flipping vertex indices within individual segments does not affect the final compressed ordering.
  - Confirms that mixed permutations and flips still yield the same canonical ordering, starting at the smaller endpoint for open polylines or the smallest participating vertex for closed polygons.

- **Polyline vs. Polygon Detection (Part B):**
  - Demonstrates correct classification of both open polylines and closed polygons.
  - Exercises both construction paths (verbose segments and compressed vertex ordering).
  - Verifies consistency between `IsPolygon()` and `GetType()` across all construction paths, including round-trip reconstruction.

- **Edge Cases and Validation:**
  - Covers minimal inputs, short and long chains, and non-contiguous (sparse) vertex index spaces.
  - Demonstrates correct handling of malformed inputs, including invalid segment buffers, excessive vertex degree, disconnected components, duplicate segments, and degenerate cases.
  - Confirms that optional vertex coordinate data is preserved without affecting polyline behavior.

- **Performance:**
  - Demonstrated using a stress test on a very long open polyline (200,000 vertices).
  - On the test system (a 10-year-old dual-core Intel CPU with 8 GB of RAM), compression completed in approximately 30 ms.
  - This confirms linear scaling behavior and shows that the implementation remains efficient even for large inputs on modest hardware.

## Answer

*Prompt: Succinct final answer or summary of deliverables (what files to run, how to run them).*

- **Deliverables:**
  - `src/problem_2/polyline.hpp` — public API (`Polyline`, `PolylineRepresentation`, `PolylineType`, `GetCompressedVertexOrdering`)
  - `src/problem_2/polyline.cpp` — implementation
  - `tests/problem_2/test_polyline.cpp` — GoogleTest suite

- **Build:** From the repository root, run `cmake -B build -S .` followed by `cmake --build build`.

- **Run tests:** Execute `ctest --test-dir build -R problem2 --output-on-failure`, or run the `problem2_tests` executable directly from the build directory.

## Rubric

*Prompt: A short self-assessment (optional) indicating what parts you believe deserve full credit and what is incomplete. Partial credit will be awarded for correct approaches and clear reasoning even if a complete implementation is not finished. Explicitly document any limitations and the most important next steps.*

- **Self-assessment:** I believe this submission deserves full credit for the following reasons:
    - The implementation is complete and production-ready.
    - Full input validation is performed in the constructor, including enforcement of degree and endpoint invariants.
    - The compressed vertex ordering is deterministic (open polylines start at the smaller endpoint; closed polygons start at the smallest participating vertex).
    - Open versus closed classification is handled consistently via `IsPolygon()` and `GetType()`.
    - Test coverage is comprehensive, including determinism, edge cases, polygons, round-trip behavior, validation, sparse vertex indices, optional vertex storage, and performance.

- **Limitations:**
  - The implementation assumes a single connected polyline or polygon, as specified in the problem. Multiple disconnected components are treated as invalid input.
  - Only simple polylines and polygons are supported, with each vertex having degree at most two. More general graph‑like structures are intentionally out of scope.
  - No geometric validation is performed beyond topological checks (e.g., no self‑intersection check).
  - Input validation prioritizes robustness and clear invariants over minimal overhead, which may introduce additional cost for extremely large (millions of vertices) inputs.

- **Next steps:**
  - Extend the data model to support multiple connected components and expose them as separate polylines when appropriate.
  - Add optional geometric validation, such as self‑intersection detection or orientation consistency checks for polygons.
  - Integrate the polyline representation into downstream geometry operations (e.g., extrusion, surface generation, or mesh construction) to validate its usefulness in a larger CAD or geometry processing pipeline.

## AI Disclosure

I used AI tools throughout the development of this solution as a productivity aid, mainly to speed up planning and test development. AI was not used to make design decisions or "vibe-code", but rather to help with well‑scoped, fine‑grained tasks where it could save time. I would describe it as "AI-assisted engineering" where I crafted a robust, readable, well-tested software faster with help from AI and hopefully it is pretty evident from the code.

- **AI tools used:**
  - I primarily used the Cursor editor with Auto mode enabled (likely backed by the Composer 1.5 model).
  - Most interactions were done in “Ask” mode rather than relying on agentic code generation. Not that there is anything wrong with that.

- **How AI was used:**
  - Early on, I used AI (Gemini and ChatGPT) to help reason through the problem statement and clarify requirements.
  - During implementation, AI was most useful for scaling up test coverage, especially for edge cases and large‑input stress tests.
  - In a few cases, I used AI to generate initial drafts of small, self‑contained code snippets (mostly test scaffolding), which I then refined manually.

- **Prompts and queries:**
  - I used many short, targeted prompts throughout development, making it impractical to list them all. I will make an attempt to list them as appendix.
  - Typical prompts were very specific and task‑oriented, for example:
    - “Implement a polyline test with 200K segments and provide reasonable assertions.”

- **How AI‑generated output was validated and adapted:**
  - I read through all generated code to make sure it was correct and aligned with my intent.
  - Generated code was often modified to improve structure, naming, and readability, and to better match my preferred coding style (largely based on Google’s C++ style guidelines).
  - I added comments where needed to clarify intent or non‑obvious behavior.
  - All changes were validated by compiling and running the full test suite to ensure correctness and consistency with the problem requirements.

Overall, I treated AI as a supporting tool to accelerate iteration, while keeping all final design decisions, code structure, and validation firmly under manual control.
