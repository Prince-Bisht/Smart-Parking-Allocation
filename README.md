
# Smart Parking Allocation and Reservation Platform

A desktop-based Smart Parking Allocation and Reservation Platform developed using **C17 and raylib**, integrating Data Structures and Design & Analysis of Algorithms (DAA).

The system helps drivers find available parking spaces, compare parking distances, manage reservations, and navigate between registered locations using graph-based routing algorithms.

## Features

### 1. Interactive Desktop GUI
- Modern dark-themed dashboard using raylib.
- Clean typography and interactive buttons.
- Parking location cards with real-time simulated occupancy.
- Separate Dashboard, Algorithm Lab, and Route Navigation screens.

### 2. Parking Management
- View parking locations and available slots.
- Display parking capacity, occupied slots, and hourly pricing.
- Automatically update occupancy after reservations and cancellations.
- Save parking information using CSV files.

### 3. Main Parking Locations

The platform manages five major parking locations in Dehradun:

| Node ID | Parking Location |
|---|---|
| 0 | Prem Nagar |
| 1 | Clock Tower |
| 2 | ISBT |
| 3 | Raipur |
| 4 | Mall of Dehradun |

### 4. Universal Location Dataset

The system supports 14 smaller locations:

- Nanda Ki Chowki
- IMA
- FRI
- Kandoli
- Bidholi
- Ballupur
- Balliwala
- Prince Chowk
- Dharampur
- Rispana
- Jogiwala
- Miyawala
- Mokhampur
- Cargi Chowk

The distance dataset contains **70 location-to-parking distance pairs**.

Distances are approximate road-distance estimates used for simulation, not fully verified live Google Maps measurements.

### 5. Nearest Parking Allocation
- Enter a registered starting location.
- Search across all five main parking locations.
- Filter parking locations based on available capacity and budget.
- Recommend the nearest feasible parking location.
- Automatically consider alternative parking when nearby locations are full.
- Support case-insensitive location searching and common spelling variations.

### 6. Parking Reservation System
- Reserve available parking slots.
- Generate unique reservation IDs.
- Cancel active reservations.
- Automatically update parking occupancy.
- Store reservation records in CSV format.
- Preserve reservation data between application runs.

### 7. Route Navigation
- Enter starting and destination locations.
- Calculate a shortest path through the modelled location graph.
- Display intermediate locations in arrow-based route format.
- Display estimated total route distance.
- Support routes between registered main and small locations.
- Use Dijkstra's shortest-path approach with parent-based path reconstruction.

Example:

Miyawala → Mokhampur → Jogiwala → Rispana → Dharampur

**Note:** Routes are generated using an approximate simulation graph. They are not verified turn-by-turn driving directions.

### 8. DAA Algorithm Laboratory

The project contains reusable implementations of multiple DAA algorithms in `algorithms.c`.

**Sorting and Searching**
- Merge Sort
- Quick Sort
- Iterative Binary Search
- Recursive Binary Search

**Divide and Conquer**
- Maximum Subarray
- Fast Exponentiation
- Celebrity Problem

**Greedy Algorithms**
- Activity Selection
- Fractional Knapsack
- Interval Partitioning
- Dijkstra's Algorithm
- Prim's Minimum Spanning Tree
- Kruskal's Minimum Spanning Tree

**Dynamic Programming**
- 0/1 Knapsack
- Longest Common Subsequence (LCS)
- Matrix Chain Multiplication Cost
- Optimal Binary Search Tree
- Bellman-Ford
- Floyd-Warshall
- Held-Karp Travelling Salesperson Problem

**Backtracking**
- N-Queens
- Subset Sum

**Graph Algorithms**
- Ford-Fulkerson Maximum Flow
- Maximum Bipartite Matching

**Number Theory**
- Chinese Remainder Theorem

**String Matching**
- Naive String Matching
- Rabin-Karp
- Finite Automaton Matching
- Knuth-Morris-Pratt (KMP)

Not all algorithms have separate GUI visualizations. Additional demonstrations are available through the terminal workbench.

### 9. Algorithm Benchmarking
- Compare Dijkstra and Bellman-Ford.
- Measure CPU execution time.
- Compare computed shortest-path distances.
- Export benchmark results to CSV.
- Support repeated algorithm executions for performance analysis.

### 10. CSV Data Management

The application uses:

- `parking.csv` — parking locations, capacity, occupancy, and prices.
- `reservations.csv` — reservation records and active status.
- `distances.csv` — approximate distances between small locations and main parking nodes.
- `benchmark.csv` — algorithm performance results.

## Project Structure

```text
Smart_Parking_Allocation/
├── main.c
├── algorithms.c
├── algorithms.h
├── assets/
│   └── font.ttf
├── data/
│   ├── parking.csv
│   ├── reservations.csv
│   └── distances.csv
├── reports/
│   └── benchmark.csv
├── tests/
│   └── TEST_PLAN.md
└── README.md
```

The custom font file is optional. The application can use a system font when available.

## Technologies Used

- Programming Language: C17
- GUI Library: raylib
- Compiler: Clang or GCC
- Data Storage: CSV
- Development Environment: Visual Studio Code
- Supported Platforms: macOS and Windows (with suitable compiler and raylib installation)

## Installation and Execution

### macOS

Install raylib:

```bash
brew install raylib pkg-config
```

Compile:

```bash
clang -std=c17 -Wall -Wextra -DUSE_RAYLIB main.c algorithms.c -o smart_parking $(pkg-config --cflags --libs raylib) -lm
```

Run:

```bash
./smart_parking
```

### CLI Mode

Compile without raylib:

```bash
clang -std=c17 -Wall -Wextra main.c algorithms.c -o smart_parking_cli
```

Run:

```bash
./smart_parking_cli
```

Run all commands from the project root directory so the application can access the CSV datasets.

## Example Demonstration

### Scenario 1: Normal Parking Allocation

Starting Location: Miyawala

Recommended Parking: Mall of Dehradun

Estimated Distance: 1.6 km

### Scenario 2: Capacity Shock

Starting Location: Miyawala

Condition: All parking locations except Prem Nagar are full.

Recommended Parking: Prem Nagar

Estimated Distance: 17.0 km

### Scenario 3: Route Navigation

Starting Location: Miyawala

Destination: Dharampur

The application displays a route through connected graph nodes using arrows and reports the modelled shortest distance.

## Data Structures Used

- Structures for parking lots and reservations.
- Arrays for storing parking information.
- Two-dimensional arrays for weighted graph representation.
- Character arrays for location names.
- Parent arrays for shortest-path reconstruction.
- Dynamic memory allocation in selected algorithms.

## Algorithm Complexity

| Algorithm | Time Complexity |
|---|---|
| Merge Sort | O(n log n) |
| Quick Sort | Average O(n log n), Worst O(n²) |
| Binary Search | O(log n) |
| Dijkstra (Adjacency Matrix) | O(V²) |
| Bellman-Ford | O(VE) |
| Floyd-Warshall | O(V³) |
| Prim (Adjacency Matrix) | O(V²) |
| Kruskal | O(E log E) |
| KMP | O(n + m) |
| 0/1 Knapsack DP | O(nW) |
| LCS | O(nm) |
| N-Queens Backtracking | Exponential worst-case |

## Project Limitations

- Parking availability is simulated rather than connected to live parking sensors.
- Road distances are approximate.
- Route navigation is graph-based and does not provide live traffic or turn-by-turn instructions.
- The application supports registered locations rather than arbitrary addresses.
- CSV storage is intended for local demonstration, not concurrent multi-user transactions.
- Benchmarking focuses primarily on shortest-path algorithms.

## Future Enhancements

- Live Google Maps or OpenStreetMap integration.
- GPS-based location detection.
- Real-time parking sensors.
- Database-backed reservations.
- User authentication.
- Dynamic traffic-aware routing.
- Interactive map visualization.
- Online booking and payment integration.

## Conclusion

The Smart Parking Allocation and Reservation Platform demonstrates how Data Structures and DAA algorithms can be applied to urban parking allocation, reservation management, shortest-path routing, and algorithm performance analysis.

The project combines a C17 application, raylib desktop GUI, CSV datasets, and reusable algorithm implementations to create an educational urban mobility simulation.
