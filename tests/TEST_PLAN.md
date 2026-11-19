# Manual test plan
1. Start application: six lots and correct aggregate occupancy shown.
2. Select a lot and reserve: available decreases by one; `data/parking.csv` and `data/reservations.csv` update.
3. Fill a lot to capacity; extra booking must be rejected.
4. Cancel latest reservation; occupancy decreases by one.
5. Find nearest parking: only lots within price budget and with free capacity qualify.
6. DAA Lab: sorting and searching produce matching index; knapsack and LCS produce values; naive and KMP return same position.
7. Run benchmark: `reports/benchmark.csv` has Dijkstra and Bellman-Ford rows, matching_distances=1 for sample nonnegative graph.
8. Test invalid or missing CSV: defaults remain when no valid lot is loaded.
9. Verify build on macOS and Windows with raylib installed.
