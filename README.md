# SmartParking C17 — Two C files

- `main.c`: parking management, CSV, CLI and optional raylib GUI.
- `algorithms.c`: DAA algorithms, separate from the application.
- `algorithms.h`: declarations connecting the two source files (not a third C source).
- `data/`: parking and reservation CSV files.
- `reports/`: benchmark output.

## macOS CLI (no dependencies)

```sh
clang -std=c17 -Wall -Wextra main.c algorithms.c -o smart_parking
./smart_parking
```

## macOS raylib GUI

```sh
brew install raylib pkg-config
clang -std=c17 -Wall -Wextra -DUSE_RAYLIB main.c algorithms.c -o smart_parking_gui $(pkg-config --cflags --libs raylib) -lm
./smart_parking_gui
```

## Windows CLI (MinGW)

```sh
gcc -std=c17 -Wall -Wextra main.c algorithms.c -o smart_parking.exe
smart_parking.exe
```

## Windows GUI (MinGW + raylib installed)

```sh
gcc -std=c17 -Wall -Wextra -DUSE_RAYLIB main.c algorithms.c -o smart_parking_gui.exe -lraylib -lopengl32 -lgdi32 -lwinmm
smart_parking_gui.exe
```

Configure raylib include/library directories for your MinGW installation when necessary.

## Important limitations

This is a working two-file prototype, **not yet a fully verified complete syllabus submission**. Implemented functions include sorting/searching, Dijkstra, Bellman-Ford, Prim, Kruskal, Floyd-Warshall, Ford-Fulkerson (Edmonds-Karp), bipartite matching, knapsack DP/fractional, matrix-chain DP, optimal BST, TSP DP, N-Queens, subset sum, activity selection, interval partitioning, CRT, naive/Rabin-Karp/finite automaton/KMP matching, and others. Missing syllabus-specific implementations include Strassen matrix multiplication, Huffman coding, knapsack backtracking and FIFO/LIFO/least-cost branch-and-bound variants. Maximum subarray uses Kadane's method rather than a divide-and-conquer implementation. Full edge-case validation, correctness proofs, systematic benchmark comparisons, and stress testing remain incomplete.

The GUI build is optional and was not compiled in this environment (raylib unavailable here). The CLI build was compiled and exercised. GUI shows six lots, uses a fixed window and the demo driver ID 101. CSV paths are relative to the working directory; run from project root. Existing occupied counts may include historical parking not represented in reservations. The file saves on exit and booking changes; it does not implement transactional/concurrent reservations.
