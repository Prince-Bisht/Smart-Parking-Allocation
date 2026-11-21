#include "algorithms.h"
#ifdef USE_RAYLIB
#include "raylib.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

#define MAX_LOTS 32
#define MAX_RES 256

typedef struct {
    int id, node, capacity, occupied, price;
    char name[48];
} Lot;

typedef struct {
    int id, lotId, driverId, active;
} Reservation;

static Lot lots[MAX_LOTS];
static Reservation bookings[MAX_RES];
static int lotCount, bookingCount, nextBooking = 1;

static void initialize(void) {
    const Lot initial[] = {
        {1,0,30,8,25,"Prem Nagar"},
        {2,1,35,12,30,"Clock Tower"},
        {3,2,40,15,20,"ISBT"},
        {4,3,25,7,25,"Raipur"},
        {5,4,45,20,35,"Mall of Dehradun"}
    };
    memcpy(lots, initial, sizeof initial);
    lotCount = 5;
    nodes = 5;
    for (int i=0; i<MAX_NODES; ++i)
        for (int j=0; j<MAX_NODES; ++j)
            graph[i][j] = i==j ? 0 : INF;
}

#define SMALL_COUNT 14
static const char *smallNames[SMALL_COUNT] = {
    "Nanda Ki Chowki", "IMA", "FRI", "Kandoli", "Bidholi",
    "Ballupur", "Balliwala", "Prince Chowk", "Dharampur",
    "Rispana", "Jogiwala", "Miyawala", "Mokhampur", "Cargi Chowk"
};
/* Estimated driving distances, in tenths of a kilometre.
   Simulation data; not a fully Google Maps-verified dataset. */
static int smallKm10[SMALL_COUNT][5];

static void loadDistances(void) {
    FILE *f = fopen("data/distances.csv", "r");
    if (!f) {
        fprintf(stderr, "Missing data/distances.csv: location search unavailable\n");
        return;
    }
    char line[256];
    if (!fgets(line, sizeof line, f)) { fclose(f); return; }
    int seen[SMALL_COUNT] = {0};
    while (fgets(line, sizeof line, f)) {
        char name[64]; double d[5];
        if (sscanf(line, "%63[^,],%lf,%lf,%lf,%lf,%lf",
                   name,&d[0],&d[1],&d[2],&d[3],&d[4]) != 6) continue;
        for (int i=0;i<SMALL_COUNT;i++) {
            if (strcmp(name,smallNames[i]) != 0) continue;
            for (int j=0;j<5;j++)
                smallKm10[i][j] = d[j] >= 0 && d[j] < 10000 ?
                    (int)(d[j]*10.0+0.5) : INF;
            seen[i] = 1;
        }
    }
    fclose(f);
    /* Infer main-node connections via shared smaller locations.
       These are modelled estimates, not surveyed roads. */
    for (int i=0;i<5;i++) for(int j=i+1;j<5;j++) {
        int best=INF;
        for (int k=0;k<SMALL_COUNT;k++) if(seen[k] &&
            smallKm10[k][i]<INF && smallKm10[k][j]<INF &&
            smallKm10[k][i]+smallKm10[k][j]<best)
            best=smallKm10[k][i]+smallKm10[k][j];
        graph[i][j]=graph[j][i]=best;
    }
}

static int sameLocation(const char *input, const char *name) {
    while (isspace((unsigned char)*input)) ++input;
    while (*input && *name) {
        if (tolower((unsigned char)*input) != tolower((unsigned char)*name)) return 0;
        ++input;
        ++name;
    }
    while (isspace((unsigned char)*input)) ++input;
    return *input == '\0' && *name == '\0';
}

static int locationIndex(const char *name) {
    for(int i=0;i<SMALL_COUNT;i++) if(sameLocation(name,smallNames[i])) return i;
    for(int i=0;i<lotCount;i++) if(sameLocation(name,lots[i].name)) return -lots[i].node-1;
    /* common spelling aliases */
    if(sameLocation(name,"Miawala")) return 11;
    if(sameLocation(name,"Mokahmpur")) return 12;
    if(sameLocation(name,"Dahrampur")) return 8;
    if(sameLocation(name,"Cargi")) return 13;
    if(sameLocation(name,"Premnagar")) return -1;
    return -100;
}
static int nearestFromName(const char *name,int budget,int *distance10) {
    int k=locationIndex(name);
    if(k==-100) return -2;
    int best=-1,bestDistance=INF;
    if(k>=0) {
        for(int i=0;i<lotCount;i++) {
            int distance=smallKm10[k][lots[i].node];
            if(lots[i].occupied>=lots[i].capacity ||
               lots[i].price>budget || distance>=INF) continue;
            if(best<0 || distance<bestDistance ||
               (distance==bestDistance && lots[i].price<lots[best].price)) {
                best=i;bestDistance=distance;
            }
        }
    } else {
        int node=-k-1,dist[MAX_NODES];
        dijkstra(node,dist);
        for(int i=0;i<lotCount;i++) {
            int distance=dist[lots[i].node];
            if(lots[i].occupied>=lots[i].capacity ||
               lots[i].price>budget || distance>=INF) continue;
            if(best<0 || distance<bestDistance ||
               (distance==bestDistance && lots[i].price<lots[best].price)) {
                best=i;bestDistance=distance;
            }
        }
    }
    if(distance10) *distance10=bestDistance;
    return best;
}

/* Illustrative road-corridor graph. Weights are approximate simulation
   values, NOT verified road segments. The 70-pair CSV remains used for
   nearest-parking recommendations; this separate graph displays waypoints. */
#define ROUTE_NODES (5 + SMALL_COUNT)
static int routeWeight[ROUTE_NODES][ROUTE_NODES];
static const char *routeName(int id) {
    return id < 5 ? lots[id].name : smallNames[id - 5];
}
static void routeEdge(int u, int v, int tenthsKm) {
    if (u >= 0 && v >= 0 && u < ROUTE_NODES && v < ROUTE_NODES &&
        tenthsKm > 0 && tenthsKm < routeWeight[u][v])
        routeWeight[u][v] = routeWeight[v][u] = tenthsKm;
}
static void makeRouteGraph(void) {
    for (int i = 0; i < ROUTE_NODES; ++i)
        for (int j = 0; j < ROUTE_NODES; ++j)
            routeWeight[i][j] = i == j ? 0 : INF;

    /* IDs 0..4: main parking; IDs 5..18: smallNames in listed order.
       Road corridors are deliberately modelled as consecutive waypoints. */
    const int corridors[][3] = {
        {11+5,12+5,15}, /* Miyawala -> Mokhampur */
        {12+5,10+5,20}, /* Mokhampur -> Jogiwala */
        {10+5, 9+5,18}, /* Jogiwala -> Rispana */
        { 9+5, 8+5,20}, /* Rispana -> Dharampur */
        { 8+5, 7+5,24}, /* Dharampur -> Prince Chowk */
        { 7+5, 6+5,23}, /* Prince Chowk -> Balliwala */
        { 6+5, 5+5,12}, /* Balliwala -> Ballupur */
        { 5+5, 2+5,20}, /* Ballupur -> FRI */
        { 2+5, 1+5,30}, /* FRI -> IMA */
        { 1+5, 0+5,20}, /* IMA -> Nanda Ki Chowki */
        { 0+5, 4+5,70}, /* Nanda Ki Chowki -> Bidholi */
        { 4+5, 3+5,40}, /* Bidholi -> Kandoli */
        { 7+5,13+5,38}, /* Prince Chowk -> Cargi Chowk */
        {13+5, 6+5,30}, /* Cargi Chowk -> Balliwala */
        { 0, 0+5,24}, /* Prem Nagar -> Nanda Ki Chowki */
        { 0, 1+5,22}, /* Prem Nagar -> IMA */
        { 1, 7+5,12}, /* Clock Tower -> Prince Chowk */
        { 1, 8+5,42}, /* Clock Tower -> Dharampur */
        { 2,13+5,20}, /* ISBT -> Cargi Chowk */
        { 2, 6+5,63}, /* ISBT -> Balliwala */
        { 3,10+5,60}, /* Raipur -> Jogiwala */
        { 3,11+5,53}, /* Raipur -> Miyawala */
        { 4,11+5,16}, /* Mall -> Miyawala */
        { 4,12+5,10}  /* Mall -> Mokhampur */
    };
    for (size_t i = 0; i < sizeof corridors / sizeof corridors[0]; ++i)
        routeEdge(corridors[i][0], corridors[i][1], corridors[i][2]);
}
static int routeIdFromName(const char *name) {
    int k = locationIndex(name);
    if (k == -100) return -1;
    return k >= 0 ? k + 5 : -k - 1;
}
/* Returns total tenths of km; -1 means unknown/unreachable.
   Caller supplies an array of ROUTE_NODES and receives ordered IDs. */
static int routeBetween(int start, int end, int path[ROUTE_NODES], int *length) {
    int dist[ROUTE_NODES], parent[ROUTE_NODES], used[ROUTE_NODES] = {0};
    *length = 0;
    if (start < 0 || end < 0 || start >= ROUTE_NODES || end >= ROUTE_NODES)
        return -1;
    for (int i=0; i<ROUTE_NODES; ++i) {
        dist[i] = INF;
        parent[i] = -1;
    }
    dist[start] = 0;
    for (int step=0; step<ROUTE_NODES; ++step) {
        int u = -1;
        for (int i=0; i<ROUTE_NODES; ++i)
            if (!used[i] && (u < 0 || dist[i] < dist[u])) u = i;
        if (u < 0 || dist[u] == INF || u == end) break;
        used[u] = 1;
        for (int v=0; v<ROUTE_NODES; ++v)
            if (!used[v] && routeWeight[u][v] < INF &&
                dist[u] + routeWeight[u][v] < dist[v]) {
                dist[v] = dist[u] + routeWeight[u][v];
                parent[v] = u;
            }
    }
    if (dist[end] == INF) return -1;
    int reversed[ROUTE_NODES], n = 0;
    for (int v=end; v!=-1 && n<ROUTE_NODES; v=parent[v]) {
        reversed[n++] = v;
        if (v == start) break;
    }
    if (!n || reversed[n-1] != start) return -1;
    for (int i=0; i<n; ++i) path[i] = reversed[n-1-i];
    *length = n;
    return dist[end];
}

static int loadLots(void) {
    FILE *f = fopen("data/parking.csv", "r");
    if (!f) return 0;
    char line[256];
    if (!fgets(line, sizeof(line), f)) { fclose(f); return 0; }
    Lot temp[MAX_LOTS];
    int count = 0;
    while (count < MAX_LOTS && fgets(line, sizeof(line), f)) {
        Lot x;
        if (sscanf(line, "%d,%d,%d,%d,%d,%47[^\r\n]",
                   &x.id, &x.node, &x.capacity, &x.occupied, &x.price, x.name) == 6 &&
            x.id > 0 && x.node >= 0 && x.node < nodes &&
            x.capacity >= 0 && x.occupied >= 0 &&
            x.occupied <= x.capacity && x.price >= 0) {
            int duplicate = 0;
            for (int j = 0; j < count; ++j)
                if (temp[j].id == x.id) duplicate = 1;
            if (!duplicate) temp[count++] = x;
        }
    }
    fclose(f);
    if (!count) return 0;
    memcpy(lots, temp, (size_t)count * sizeof(Lot));
    lotCount = count;
    return 1;
}

static void saveLots(void) {
    FILE *f = fopen("data/parking.csv", "w");
    if (!f) { puts("Warning: cannot save parking.csv"); return; }
    fprintf(f, "id,node,capacity,occupied,price,name\n");
    for (int i = 0; i < lotCount; ++i)
        fprintf(f, "%d,%d,%d,%d,%d,%s\n", lots[i].id, lots[i].node,
                lots[i].capacity, lots[i].occupied, lots[i].price, lots[i].name);
    fclose(f);
}

static void saveBookings(void) {
    FILE *f = fopen("data/reservations.csv", "w");
    if (!f) { puts("Warning: cannot save reservations.csv"); return; }
    fprintf(f, "reservation_id,lot_id,driver_id,active\n");
    for (int i = 0; i < bookingCount; ++i)
        fprintf(f, "%d,%d,%d,%d\n", bookings[i].id, bookings[i].lotId,
                bookings[i].driverId, bookings[i].active);
    fclose(f);
}

static void loadBookings(void) {
    FILE *f = fopen("data/reservations.csv", "r");
    if (!f) return;
    char line[256];
    if (!fgets(line, sizeof(line), f)) { fclose(f); return; }
    while (bookingCount < MAX_RES && fgets(line, sizeof(line), f)) {
        Reservation r;
        if (sscanf(line, "%d,%d,%d,%d", &r.id, &r.lotId,
                   &r.driverId, &r.active) != 4 || r.id <= 0 ||
            (r.active != 0 && r.active != 1)) continue;
        int found = 0, duplicate = 0;
        for (int j = 0; j < lotCount; ++j)
            if (lots[j].id == r.lotId) found = 1;
        for (int j = 0; j < bookingCount; ++j)
            if (bookings[j].id == r.id) duplicate = 1;
        if (!found || duplicate) continue;
        bookings[bookingCount++] = r;
        if (r.id >= nextBooking) nextBooking = r.id + 1;
    }
    fclose(f);
}


static int reserve(int idx, int driver) {
    if (idx < 0 || idx >= lotCount || driver <= 0 ||
        lots[idx].occupied >= lots[idx].capacity ||
        bookingCount >= MAX_RES) return -1;
    int id = nextBooking++;
    bookings[bookingCount++] = (Reservation){id, lots[idx].id, driver, 1};
    lots[idx].occupied++;
    saveLots();
    saveBookings();
    return id;
}

static int cancel(int reservationId) {
    for (int i = 0; i < bookingCount; ++i) {
        if (bookings[i].id != reservationId || !bookings[i].active) continue;
        for (int j = 0; j < lotCount; ++j) {
            if (lots[j].id == bookings[i].lotId) {
                if (lots[j].occupied <= 0) return 0;
                lots[j].occupied--;
                bookings[i].active = 0;
                saveLots();
                saveBookings();
                return 1;
            }
        }
    }
    return 0;
}

static int readInt(const char *prompt, int *value) {
    char line[128], extra;
    printf("%s", prompt);
    if (!fgets(line, sizeof(line), stdin)) return 0;
    return sscanf(line, " %d %c", value, &extra) == 1;
}

static void viewLots(void) {
    puts("\nID  Parking location          Node  Free/Total  Rs/hr");
    puts("--------------------------------------------------------");
    for (int i = 0; i < lotCount; ++i)
        printf("%-3d %-25s %-5d %3d/%-6d %d\n", lots[i].id,
               lots[i].name, lots[i].node,
               lots[i].capacity - lots[i].occupied,
               lots[i].capacity, lots[i].price);
}

static void occupancyReport(void) {
    int cap = 0, occupied = 0;
    for (int i = 0; i < lotCount; ++i) {
        cap += lots[i].capacity;
        occupied += lots[i].occupied;
    }
    printf("\nLocations: %d | Total: %d | Occupied: %d | Available: %d\n",
           lotCount, cap, occupied, cap - occupied);
    printf("Occupancy rate: %.2f%%\n", cap ? 100.0 * occupied / cap : 0.0);
}

static void viewBookings(void) {
    puts("\nReservation  Lot  Driver  Status");
    puts("---------------------------------");
    for (int i = 0; i < bookingCount; ++i)
        printf("%-12d %-4d %-7d %s\n", bookings[i].id,
               bookings[i].lotId, bookings[i].driverId,
               bookings[i].active ? "Active" : "Cancelled");
    if (!bookingCount) puts("No reservations recorded.");
}

static void runWorkbench(void) {
    int a[] = {8, 3, 9, 1, 5, 2}, tmp[6];
    mergeSort(a, tmp, 0, 5);
    printf("\nMerge sort: ");
    for (int i = 0; i < 6; ++i) printf("%d ", a[i]);
    printf("\nBinary search for 5: iterative=%d, recursive=%d\n",
           binaryIter(a, 6, 5), binaryRec(a, 0, 5, 5));
    int b[] = {8, 3, 9, 1, 5, 2};
    quickSort(b, 0, 5);
    printf("Quick sort: ");
    for (int i = 0; i < 6; ++i) printf("%d ", b[i]);
    int c[] = {-2, 3, -1, 5, -4};
    printf("\nMaximum subarray: %d\n", maxSubarray(c, 5));
    printf("Fast power 2^10: %lld\n", fastPower(2, 10));
    const int knows[4][4] = {{0,1,1,1},{0,0,1,1},{0,0,0,0},{0,0,1,0}};
    printf("Celebrity index: %d\n", celebrity(knows, 4));
    const int starts[] = {1,3,0,5,8}, ends[] = {2,4,6,7,9};
    printf("Activity selection: %d\n", activitySelection(starts, ends, 5));
    const int weights[] = {2,3,4}, values[] = {4,5,7};
    printf("0/1 knapsack: %d\n", knapsack01(weights, values, 3, 5));
    printf("LCS(PARKING,KING): %d\n", lcs("PARKING", "KING"));
    const char *text = "parking reservation parking";
    printf("Naive matching index: %d\n", naiveMatch(text, "reservation"));
    printf("KMP matching index: %d\n", kmpMatch(text, "reservation"));
    printf("Rabin-Karp matching index: %d\n", rabinKarp(text, "reservation"));
    printf("Finite automaton matching index: %d\n", finiteAutomatonMatch(text, "reservation"));
    printf("Prim MST total: %d\n", primMST(nodes, (const int (*)[MAX_NODES])graph));
    printf("Kruskal MST total: %d\n", kruskalMST(nodes, (const int (*)[MAX_NODES])graph));
    int allPairs[MAX_NODES][MAX_NODES];
    floydWarshall(nodes, (const int (*)[MAX_NODES])graph, allPairs);
    printf("Floyd-Warshall distance 0->5: %d\n", allPairs[0][5]);
    printf("TSP DP (6 nodes): %d\n", tspHeldKarp(nodes, (const int (*)[MAX_NODES])graph));
    printf("N-Queens (4): %d solutions\n", nQueensCount(4));
    int subset[] = {3, 5, 7};
    printf("Subset sum target 8: %d\n", subsetSumExists(subset, 3, 8));
    int dims[] = {10, 20, 30, 40};
    printf("Matrix chain cost: %d\n", matrixChainCost(dims, 4));
    int frequencies[] = {3, 2, 4};
    printf("Optimal BST cost: %d\n", optimalBSTCost(frequencies, 3));
    int rem[] = {2, 3, 2}, mod[] = {3, 5, 7};
    printf("CRT result: %lld\n", chineseRemainder(rem, mod, 3));
    printf("Fractional knapsack: %.2f\n", fractionalKnapsack(weights, values, 3, 5));
    printf("Interval partitioning: %d rooms\n", intervalPartitionCount(starts, ends, 5));
    int flowNetwork[MAX_NODES][MAX_NODES] = {{0}};
    flowNetwork[0][1] = 3; flowNetwork[0][2] = 2;
    flowNetwork[1][3] = 2; flowNetwork[2][3] = 3;
    printf("Ford-Fulkerson maximum flow: %d\n", maxFlow(4, (const int (*)[MAX_NODES])flowNetwork, 0, 3));
    int assignments[MAX_NODES][MAX_NODES] = {{0}};
    assignments[0][0]=1;assignments[0][1]=1;assignments[1][1]=1;
    printf("Bipartite matching: %d\n", bipartiteMatching(2, 2, (const int (*)[MAX_NODES])assignments));

}

static void benchmark(void) {
    int a[MAX_NODES], b[MAX_NODES], same = 1;
    volatile int sink = 0;
    const int repetitions = 10000;
    clock_t start = clock();
    for (int i = 0; i < repetitions; ++i) {
        dijkstra(0, a);
        sink ^= a[i % nodes];
    }
    double dMs = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;
    start = clock();
    for (int i = 0; i < repetitions; ++i) {
        bellmanFord(0, b);
        sink ^= b[i % nodes];
    }
    double bMs = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;
    for (int i = 0; i < nodes; ++i)
        if (a[i] != b[i]) same = 0;
    printf("\nDijkstra: %.3f ms / %d runs\n", dMs, repetitions);
    printf("Bellman-Ford: %.3f ms / %d runs\n", bMs, repetitions);
    printf("Matching shortest distances: %s\n", same ? "YES" : "NO");
    FILE *f = fopen("reports/benchmark.csv", "w");
    if (!f) { puts("Could not save reports/benchmark.csv"); return; }
    fprintf(f, "algorithm,vertices,runs,total_cpu_ms,matching_distances\n");
    fprintf(f, "Dijkstra,%d,%d,%.6f,%d\n", nodes, repetitions, dMs, same);
    fprintf(f, "Bellman-Ford,%d,%d,%.6f,%d\n", nodes, repetitions, bMs, same);
    fclose(f);
    (void)sink;
    puts("Saved reports/benchmark.csv");
}

static void routeReport(void) {
    int source, a[MAX_NODES], b[MAX_NODES];
    if (!readInt("Source node (0-5): ", &source) ||
        source < 0 || source >= nodes) {
        puts("Invalid source node."); return;
    }
    dijkstra(source, a);
    bellmanFord(source, b);
    puts("Node   Dijkstra   Bellman-Ford");
    for (int i = 0; i < nodes; ++i)
        printf("%-6d %-10d %d\n", i, a[i], b[i]);
}

static int runCli(void) {
    initialize();
    loadDistances();
    loadLots();
    makeRouteGraph();
    loadBookings();
    for (;;) {
        int choice;
        puts("\n=========================================");
        puts(" SMART PARKING ALLOCATION SYSTEM (C17)");
        puts("=========================================");
        puts("1. View parking locations");
        puts("2. Find nearest available parking");
        puts("3. Reserve parking");
        puts("4. Cancel reservation");
        puts("5. Occupancy report");
        puts("6. View reservations");
        puts("7. Compare shortest routes");
        puts("8. DAA algorithm demonstrations");
        puts("9. Run routing benchmark (CSV)");
        puts("10. Show route between locations");
        puts("0. Exit");
        if (!readInt("Enter choice: ", &choice)) {
            puts("Invalid input or end of input. Exiting.");
            break;
        }
        if (choice == 0) break;
        if (choice == 1) viewLots();
        else if (choice == 2) {
            char location[80]; int budget, distance10=INF;
            printf("Location name: ");
            if(!fgets(location,sizeof location,stdin)) break;
            location[strcspn(location,"\r\n")]=0;
            if(!readInt("Max price per hour: ",&budget) || budget<0) {
                puts("Invalid budget."); continue;
            }
            int idx=nearestFromName(location,budget,&distance10);
            if(idx==-2) puts("Unknown location. Check data/distances.csv.");
            else if(idx<0) puts("No available parking within budget.");
            else printf("Recommended: %s (ID %d, %.1f km, Rs %d/hr)\n",
                        lots[idx].name,lots[idx].id,distance10/10.0,lots[idx].price);
        } else if (choice == 3) {
            int id, driver, idx = -1;
            viewLots();
            if (!readInt("Parking ID: ", &id) ||
                !readInt("Driver ID (positive): ", &driver)) {
                puts("Invalid input."); continue;
            }
            for (int i = 0; i < lotCount; ++i)
                if (lots[i].id == id) idx = i;
            int reservationId = reserve(idx, driver);
            if (reservationId < 0) puts("Reservation rejected.");
            else printf("Reservation successful: #%d\n", reservationId);
        } else if (choice == 4) {
            int id;
            if (!readInt("Reservation ID: ", &id)) puts("Invalid input.");
            else puts(cancel(id) ? "Reservation cancelled." : "No active reservation found.");
        } else if (choice == 5) occupancyReport();
        else if (choice == 6) viewBookings();
        else if (choice == 7) routeReport();
        else if (choice == 8) runWorkbench();
        else if (choice == 9) benchmark();
        else if (choice == 10) {
            char from[80], to[80]; int path[ROUTE_NODES], count=0;
            printf("From: ");
            if (!fgets(from,sizeof from,stdin)) break;
            from[strcspn(from,"\r\n")]=0;
            printf("To: ");
            if (!fgets(to,sizeof to,stdin)) break;
            to[strcspn(to,"\r\n")]=0;
            int distance=routeBetween(routeIdFromName(from),routeIdFromName(to),path,&count);
            if (distance<0) puts("Unknown or unreachable location.");
            else {
                printf("Modelled route (%.1f km): ",distance/10.0);
                for(int i=0;i<count;i++) printf("%s%s",i?" -> ":"",routeName(path[i]));
                puts("\nNote: simulated graph route, not actual road directions.");
            }
        }
        else puts("Unknown menu choice.");
    }
    saveLots();
    saveBookings();
    puts("Goodbye!");
    return 0;
}

#ifdef USE_RAYLIB
static const Color NAVY = {12, 19, 34, 255};
static const Color CARD = {25, 38, 58, 255};
static const Color TEAL = {47, 207, 157, 255};
static const Color MUTED = {170, 184, 204, 255};

/* Font-only update: preserve the existing GUI and all algorithm calls. */
static Font modernFont;
static int modernFontLoaded = 0;

static void drawModernText(const char *text, int x, int y, int size, Color color) {
    if (modernFontLoaded) {
        DrawTextEx(modernFont, text, (Vector2){(float)x, (float)y},
                   (float)size, 1.0f, color);
    } else {
        DrawText(text, x, y, size, color);
    }
}

static void loadModernFont(void) {
    const char *paths[] = {
        "assets/font.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf"
    };
    for (size_t i = 0; i < sizeof(paths)/sizeof(paths[0]); ++i) {
        if (FileExists(paths[i])) {
            modernFont = LoadFontEx(paths[i], 48, NULL, 0);
            if (modernFont.texture.id != 0) {
                modernFontLoaded = 1;
                SetTextureFilter(modernFont.texture, TEXTURE_FILTER_BILINEAR);
                break;
            }
        }
    }
}

/* Location name input for nearest-parking search. */
static char locationInput[48] = "";
static int locationFocused = 0;
static char budgetInput[12] = "60";
static int budgetFocused = 0;

static void drawBudgetInput(void) {
    Rectangle box = {260, 587, 235, 32};
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        budgetFocused = CheckCollisionPointRec(GetMousePosition(), box);
    if (budgetFocused) {
        int key;
        while ((key = GetCharPressed()) != 0) {
            size_t len = strlen(budgetInput);
            if (key >= '0' && key <= '9' && len + 1 < sizeof budgetInput) {
                budgetInput[len] = (char)key;
                budgetInput[len + 1] = '\0';
            }
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            size_t len = strlen(budgetInput);
            if (len) budgetInput[len - 1] = '\0';
        }
    }
    DrawRectangleRounded(box, 0.18f, 8, (Color){31,48,69,255});
    DrawRectangleRoundedLinesEx(box, 0.18f, 8, 1.2f,
                                budgetFocused ? TEAL : MUTED);
    drawModernText(TextFormat("Budget Rs/hr: %s", budgetInput[0] ? budgetInput : "(enter amount)"),
                   270, 592, 17, budgetInput[0] ? RAYWHITE : MUTED);
}



static void drawLocationInput(void) {
    Rectangle box = {515, 587, 490, 32};
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        locationFocused = CheckCollisionPointRec(GetMousePosition(), box);
    }
    if (locationFocused) {
        int key;
        while ((key = GetCharPressed()) != 0) {
            size_t len = strlen(locationInput);
            if (key >= 32 && key <= 126 && len + 1 < sizeof(locationInput)) {
                locationInput[len] = (char)key;
                locationInput[len + 1] = '\0';
            }
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            size_t len = strlen(locationInput);
            if (len > 0) locationInput[len - 1] = '\0';
        }
    }
    DrawRectangleRounded(box, 0.18f, 8, (Color){31, 48, 69, 255});
    DrawRectangleRoundedLinesEx(box, 0.18f, 8, 1.2f,
                                locationFocused ? TEAL : MUTED);
    drawModernText(locationInput[0] ? locationInput : "Type location: ISBT, Ballupur...",
                   525, 592, 17, locationInput[0] ? RAYWHITE : MUTED);
}

static int guiButton(Rectangle r, const char *label);
static char destinationInput[48] = "Dharampur";
static int destinationFocused = 0;
static int shownPath[ROUTE_NODES], shownLength = 0, shownDistance = -1;
static char shownFrom[48] = "", shownTo[48] = "";

static void textField(Rectangle box, char *buffer, size_t capacity, int *focused,
                      const char *placeholder) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        *focused = CheckCollisionPointRec(GetMousePosition(), box);
    if (*focused) {
        int key;
        while ((key = GetCharPressed()) != 0) {
            size_t len = strlen(buffer);
            if (key >= 32 && key <= 126 && len + 1 < capacity) {
                buffer[len] = (char)key;
                buffer[len+1] = '\0';
            }
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            size_t len = strlen(buffer);
            if (len) buffer[len-1] = '\0';
        }
    }
    DrawRectangleRounded(box, 0.16f, 8, (Color){31,48,69,255});
    DrawRectangleRoundedLinesEx(box, 0.16f, 8, 1.2f, *focused ? TEAL : MUTED);
    drawModernText(buffer[0] ? buffer : placeholder,
                   (int)box.x+10, (int)box.y+9, 17,
                   buffer[0] ? RAYWHITE : MUTED);
}
static void calculateShownRoute(const char *from, const char *to) {
    snprintf(shownFrom, sizeof shownFrom, "%s", from);
    snprintf(shownTo, sizeof shownTo, "%s", to);
    shownDistance = routeBetween(routeIdFromName(from), routeIdFromName(to),
                                 shownPath, &shownLength);
}
static void routePage(void) {
    drawModernText("Route Navigation", 260, 35, 31, RAYWHITE);
    drawModernText("Modelled route through registered locations", 260, 82, 17, MUTED);
    drawModernText("From", 260, 140, 18, RAYWHITE);
    textField((Rectangle){345,132,350,43}, locationInput,
              sizeof locationInput, &locationFocused, "Starting location");
    drawModernText("To", 260, 197, 18, RAYWHITE);
    textField((Rectangle){345,189,350,43}, destinationInput,
              sizeof destinationInput, &destinationFocused, "Destination");
    if (guiButton((Rectangle){730,151,240,53}, "Show route") ||
        ((locationFocused || destinationFocused) && IsKeyPressed(KEY_ENTER))) {
        calculateShownRoute(locationInput, destinationInput);
    }
    DrawRectangleRounded((Rectangle){260,265,850,405},0.05f,8,CARD);
    if (shownDistance < 0) {
        drawModernText("Enter two registered locations and select Show route.",
                       283,291,18,MUTED);
    } else {
        drawModernText(TextFormat("Estimated graph distance: %.1f km",shownDistance/10.0),
                       283,290,22,TEAL);
        /* Arrow-connected route chips, wrapping into additional rows. */
        int x = 284, y = 347;
        for (int i = 0; i < shownLength; ++i) {
            const char *name = routeName(shownPath[i]);
            int chipWidth = MeasureText(name, 18) + 34;
            if (modernFontLoaded)
                chipWidth = (int)MeasureTextEx(modernFont, name, 18, 1).x + 34;
            if (x + chipWidth > 1085) { x = 284; y += 68; }
            if (y > 600) {
                drawModernText("Additional waypoints omitted from display", 290, 628, 15, MUTED);
                break;
            }
            Rectangle chip = {(float)x, (float)y, (float)chipWidth, 43};
            DrawRectangleRounded(chip, 0.18f, 8,
                i == 0 || i == shownLength - 1 ?
                (Color){24,80,76,255} : (Color){35,54,76,255});
            drawModernText(name, x + 17, y + 12, 18, RAYWHITE);
            x += chipWidth + 8;
            if (i + 1 < shownLength) {
                if (x + 31 > 1085) { x = 284; y += 68; }
                drawModernText("->", x, y + 11, 18, TEAL);
                x += 35;
            }
        }
    }
    drawModernText("Approximate waypoint simulation - not verified road directions.",
                   265,680,15,MUTED);
}

static int guiButton(Rectangle r, const char *label) {
    int hover = CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRounded(r, 0.18f, 10,
        hover ? (Color){39, 64, 85, 255} : CARD);
    DrawRectangleRoundedLinesEx(r, 0.18f, 10, 1.0f, TEAL);
    drawModernText(label, (int)r.x + 15, (int)r.y + 14, 18, RAYWHITE);
    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void runGui(void) {
    initialize();
    loadDistances();
    loadLots();
    makeRouteGraph();
    loadBookings();
    InitWindow(1180, 740, "SmartPark | C17 DAA Platform");
    SetTargetFPS(60);
    loadModernFont();
    int page = 0, chosen = 0;
    char notice[160] = "Ready. Select a parking location.";
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(NAVY);
        DrawRectangle(0, 0, 225, 740, (Color){17, 28, 46, 255});
        drawModernText("SMARTPARK", 22, 32, 27, TEAL);
        drawModernText("DAA PARKING SYSTEM", 22, 72, 13, MUTED);
        if (guiButton((Rectangle){18, 135, 190, 49}, "Dashboard")) page = 0;
        if (guiButton((Rectangle){18, 202, 190, 49}, "Route Navigation")) page = 2;
        drawModernText("C17 + raylib", 24, 672, 17, MUTED);
        if (page == 0) {
            int capacity = 0, occupied = 0;
            for (int i = 0; i < lotCount; ++i) {
                capacity += lots[i].capacity;
                occupied += lots[i].occupied;
            }
            drawModernText("Parking Dashboard", 260, 35, 32, RAYWHITE);
            drawModernText("City-wide parking allocation and occupancy", 260, 82, 17, MUTED);
            const char *titles[] = {"TOTAL", "AVAILABLE", "OCCUPIED"};
            int values[] = {capacity, capacity - occupied, occupied};
            for (int i = 0; i < 3; ++i) {
                int x = 260 + 300*i;
                DrawRectangleRounded((Rectangle){(float)x,125,280,100}, .15f, 8, CARD);
                drawModernText(titles[i], x+20, 146, 17, MUTED);
                drawModernText(TextFormat("%d", values[i]), x+20, 177, 32, TEAL);
            }
            drawModernText("Select parking location", 260, 259, 24, RAYWHITE);
            for (int i = 0; i < lotCount && i < 6; ++i) {
                int x = 260 + (i%3)*300;
                int y = 310 + (i/3)*145;
                Rectangle r = {(float)x,(float)y,280,126};
                int hover = CheckCollisionPointRec(GetMousePosition(), r);
                DrawRectangleRounded(r, .14f, 8, i==chosen ?
                    (Color){28, 83, 79, 255} : CARD);
                if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) chosen = i;
                drawModernText(lots[i].name, x+14, y+15, 20, RAYWHITE);
                drawModernText(TextFormat("Free %d / %d", lots[i].capacity-lots[i].occupied,
                    lots[i].capacity), x+14, y+54, 18, TEAL);
                drawModernText(TextFormat("Rs %d / hour", lots[i].price), x+14, y+86, 16, MUTED);
            }
            if (guiButton((Rectangle){260,624,235,52}, "Reserve selected")) {
                int id = reserve(chosen, 101);
                if (id < 0) snprintf(notice,sizeof notice,"Reservation failed");
                else snprintf(notice,sizeof notice,"Reservation created: #%d",id);
            }
            drawBudgetInput();
            drawLocationInput();
            if (guiButton((Rectangle){515,624,235,52}, "Find nearest") ||
                ((locationFocused || budgetFocused) && IsKeyPressed(KEY_ENTER))) {
                char *endptr;
                long enteredBudget = strtol(budgetInput, &endptr, 10);
                if (!budgetInput[0] || *endptr != '\0' ||
                    enteredBudget < 0 || enteredBudget > 1000000) {
                    snprintf(notice, sizeof notice, "Enter a valid budget (0 to 1000000 Rs/hr).");
                } else {
                int distance10=INF;
                int idx=nearestFromName(locationInput,(int)enteredBudget,&distance10);
                if(idx==-2) {
                    snprintf(notice,sizeof notice,"Unknown location. Try Miyawala or Ballupur.");
                } else if(idx<0) {
                    snprintf(notice,sizeof notice,"No available parking within Rs %ld/hour.", enteredBudget);
                } else {
                    chosen=idx;
                    calculateShownRoute(locationInput,lots[idx].name);
                    page=2;
                    snprintf(notice,sizeof notice,"Nearest to %s: %s (%.1f km)",
                             locationInput,lots[idx].name,distance10/10.0);
                }
                }
            }
            if (guiButton((Rectangle){770,624,235,52}, "Cancel latest")) {
                int id = -1;
                for (int i = bookingCount-1; i>=0; --i) {
                    if (bookings[i].active) { id = bookings[i].id; break; }
                }
                snprintf(notice,sizeof notice, "%s", id>=0 && cancel(id) ?
                    "Reservation cancelled" : "No active reservation");
            }
        } else if (page == 2) {
            routePage();
        }
        DrawRectangle(225, 700, 955, 40, CARD);
        drawModernText(notice, 245, 711, 18, RAYWHITE);
        EndDrawing();
    }
    saveLots(); saveBookings();
    if (modernFontLoaded) UnloadFont(modernFont);
    CloseWindow();
}
#endif

int main(void) {
#ifdef USE_RAYLIB
    runGui();
    return 0;
#else
    return runCli();
#endif
}
