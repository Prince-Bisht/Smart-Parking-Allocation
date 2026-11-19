# SmartParking — single-file C17 raylib GUI

A desktop parking simulator and **partial DAA workbench**. All C source is in `smart_parking.c`. No Makefile or CMakeLists.txt is included.

## Features
- Parking location dashboard, available/occupied counts, price and capacity
- Select and reserve a parking lot; cancel the latest active reservation
- Greedy nearest feasible parking selection using Dijkstra road distances
- CSV parking persistence, reservation export, route benchmarking
- Implemented educational algorithms: linear scan, Dijkstra, Bellman-Ford, merge sort, quick sort, iterative/recursive binary search, maximum subarray (Kadane), fast exponentiation, celebrity identification, activity selection (finish-sorted input), 0/1 knapsack DP, LCS DP, naive string matching, KMP.

**Important scope:** This is NOT the full mandatory DAA syllabus. Missing examples include Strassen, Huffman, Prim, Kruskal, matrix chain, optimal BST, Floyd-Warshall, TSP, N-Queens, subset sum, branch-and-bound variants, Ford-Fulkerson, bipartite matching, CRT, Rabin-Karp, finite automata, and full analysis documentation. It is not suitable to claim full syllabus completion.

## macOS
Install Homebrew, raylib and pkg-config, then from this folder:

```sh
brew install raylib pkg-config
clang -std=c17 -Wall -Wextra smart_parking.c -o smart_parking $(pkg-config --cflags --libs raylib)
./smart_parking
```

## Windows (MSYS2 UCRT64 shell)
Install MSYS2, then in UCRT64 terminal:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib mingw-w64-ucrt-x86_64-pkgconf
 gcc -std=c17 -Wall -Wextra smart_parking.c -o smart_parking.exe $(pkg-config --cflags --libs raylib)
./smart_parking.exe
```

Use the project root as working directory so `data/` and `reports/` paths resolve. `benchmark.csv` contains runtime for 10,000 executions per algorithm, **not** per-call time. Benchmarks are simple demonstrations, not rigorous research-grade measurements. `reservations.csv` is exported, not reloaded on startup; initial occupancy is loaded from `parking.csv`. The app does not implement time-based reservation conflict checks.

## Learning and verification
Read the functions in `smart_parking.c` and test small inputs independently. Use `tests/TEST_PLAN.md` for manual cases. No precompiled executable is bundled because raylib must be installed for the target OS.
