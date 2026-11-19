
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define MAX_LOTS 32
#define MAX_RES 256
#define MAX_NODES 32
#define INF 100000000

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
static int graph[MAX_NODES][MAX_NODES];
static int nodes = 6;

static void initialize(void) {
    const Lot initial[] = {
        {1,0,8,2,25,"Clock Tower"},
        {2,1,6,1,35,"Rajpur Road"},
        {3,2,10,4,20,"ISBT"},
        {4,3,5,0,40,"Prem Nagar"},
        {5,4,7,3,30,"Ballupur"},
        {6,5,9,1,15,"Sahastradhara"}
    };

    const int edges[][3] = {
        {0,1,4}, {0,2,7}, {0,4,3},
        {1,3,5}, {1,5,8}, {2,3,4},
        {2,4,2}, {3,5,3}, {4,5,6}
    };

    memcpy(lots, initial, sizeof(initial));
    lotCount = (int)(sizeof(initial) / sizeof(initial[0]));

    for (int i = 0; i < MAX_NODES; ++i) {
        for (int j = 0; j < MAX_NODES; ++j) {
            graph[i][j] = (i == j) ? 0 : INF;
        }
    }

    for (size_t i = 0; i < sizeof(edges)/sizeof(edges[0]); ++i) {
        int u = edges[i][0];
        int v = edges[i][1];
        int w = edges[i][2];

        graph[u][v] = graph[v][u] = w;
    }
}

static int loadLots(void) {
    FILE *f = fopen("data/parking.csv", "r");

    if (!f) return 0;

    char line[256];

    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return 0;
    }

    Lot temp[MAX_LOTS];
    int count = 0;

    while (count < MAX_LOTS && fgets(line, sizeof(line), f)) {
        Lot x;

        if (sscanf(line, "%d,%d,%d,%d,%d,%47[^\r\n]",
                   &x.id, &x.node, &x.capacity,
                   &x.occupied, &x.price, x.name) == 6 &&
            x.id > 0 &&
            x.node >= 0 &&
            x.node < nodes &&
            x.capacity >= 0 &&
            x.occupied >= 0 &&
            x.occupied <= x.capacity &&
            x.price >= 0) {

            int duplicate = 0;

            for (int j = 0; j < count; ++j) {
                if (temp[j].id == x.id) {
                    duplicate = 1;
                }
            }

            if (!duplicate) {
                temp[count++] = x;
            }
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

    if (!f) {
        puts("Warning: cannot save parking.csv");
        return;
    }

    fprintf(f, "id,node,capacity,occupied,price,name\n");

    for (int i = 0; i < lotCount; ++i) {
        fprintf(f, "%d,%d,%d,%d,%d,%s\n",
                lots[i].id,
                lots[i].node,
                lots[i].capacity,
                lots[i].occupied,
                lots[i].price,
                lots[i].name);
    }

    fclose(f);
}

static void saveBookings(void) {
    FILE *f = fopen("data/reservations.csv", "w");

    if (!f) {
        puts("Warning: cannot save reservations.csv");
        return;
    }

    fprintf(f, "reservation_id,lot_id,driver_id,active\n");

    for (int i = 0; i < bookingCount; ++i) {
        fprintf(f, "%d,%d,%d,%d\n",
                bookings[i].id,
                bookings[i].lotId,
                bookings[i].driverId,
                bookings[i].active);
    }

    fclose(f);
}

static void loadBookings(void) {
    FILE *f = fopen("data/reservations.csv", "r");

    if (!f) return;

    char line[256];

    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return;
    }

    while (bookingCount < MAX_RES &&
           fgets(line, sizeof(line), f)) {

        Reservation r;

        if (sscanf(line, "%d,%d,%d,%d",
                   &r.id, &r.lotId,
                   &r.driverId, &r.active) != 4 ||
            r.id <= 0 ||
            (r.active != 0 && r.active != 1)) {
            continue;
        }

        int found = 0;
        int duplicate = 0;

        for (int j = 0; j < lotCount; ++j) {
            if (lots[j].id == r.lotId) {
                found = 1;
            }
        }

        for (int j = 0; j < bookingCount; ++j) {
            if (bookings[j].id == r.id) {
                duplicate = 1;
            }
        }

        if (!found || duplicate) continue;

        bookings[bookingCount++] = r;

        if (r.id >= nextBooking) {
            nextBooking = r.id + 1;
        }
    }

    fclose(f);
}

/* DIJKSTRA ALGORITHM */

static void dijkstra(int source, int dist[MAX_NODES]) {
    int used[MAX_NODES] = {0};

    for (int i = 0; i < nodes; ++i) {
        dist[i] = INF;
    }

    if (source < 0 || source >= nodes) return;

    dist[source] = 0;

    for (int step = 0; step < nodes; ++step) {
        int u = -1;

        for (int i = 0; i < nodes; ++i) {
            if (!used[i] &&
                (u == -1 || dist[i] < dist[u])) {
                u = i;
            }
        }

        if (u == -1 || dist[u] == INF) break;

        used[u] = 1;

        for (int v = 0; v < nodes; ++v) {
            if (!used[v] &&
                graph[u][v] < INF &&
                dist[v] > dist[u] + graph[u][v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }
}

/* BELLMAN-FORD ALGORITHM */

static void bellmanFord(int source, int dist[MAX_NODES]) {
    for (int i = 0; i < nodes; ++i) {
        dist[i] = INF;
    }

    if (source < 0 || source >= nodes) return;

    dist[source] = 0;

    for (int step = 1; step < nodes; ++step) {
        int changed = 0;

        for (int u = 0; u < nodes; ++u) {
            for (int v = 0; v < nodes; ++v) {
                if (dist[u] < INF &&
                    graph[u][v] < INF &&
                    dist[v] > dist[u] + graph[u][v]) {

                    dist[v] = dist[u] + graph[u][v];
                    changed = 1;
                }
            }
        }

        if (!changed) break;
    }
}

/* GREEDY PARKING ALLOCATION */

static int nearest(int source, int budget, int useBellman) {
    int dist[MAX_NODES];

    if (useBellman) {
        bellmanFord(source, dist);
    } else {
        dijkstra(source, dist);
    }

    int best = -1;

    for (int i = 0; i < lotCount; ++i) {
        if (lots[i].occupied >= lots[i].capacity ||
            lots[i].price > budget ||
            dist[lots[i].node] == INF) {
            continue;
        }

        if (best < 0 ||
            dist[lots[i].node] < dist[lots[best].node] ||
            (dist[lots[i].node] == dist[lots[best].node] &&
             lots[i].price < lots[best].price)) {
            best = i;
        }
    }

    return best;
}

/* RESERVATION MANAGEMENT */

static int reserve(int idx, int driver) {
    if (idx < 0 ||
        idx >= lotCount ||
        driver <= 0 ||
        lots[idx].occupied >= lots[idx].capacity ||
        bookingCount >= MAX_RES) {
        return -1;
    }

    int id = nextBooking++;

    bookings[bookingCount++] = (Reservation){
        id, lots[idx].id, driver, 1
    };

    lots[idx].occupied++;

    saveLots();
    saveBookings();

    return id;
}

static int cancel(int reservationId) {
    for (int i = 0; i < bookingCount; ++i) {
        if (bookings[i].id != reservationId ||
            !bookings[i].active) {
            continue;
        }

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

/* MERGE SORT */

static void merge(int a[], int tmp[],
                  int l, int m, int r) {
    int i = l;
    int j = m + 1;
    int k = l;

    while (i <= m && j <= r) {
        tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    }

    while (i <= m) tmp[k++] = a[i++];
    while (j <= r) tmp[k++] = a[j++];

    for (i = l; i <= r; ++i) {
        a[i] = tmp[i];
    }
}

static void mergeSort(int a[], int tmp[], int l, int r) {
    if (l >= r) return;

    int m = l + (r - l) / 2;

    mergeSort(a, tmp, l, m);
    mergeSort(a, tmp, m + 1, r);
    merge(a, tmp, l, m, r);
}

/* QUICK SORT */

static void quickSort(int a[], int l, int r) {
    if (l >= r) return;

    int pivot = a[r];
    int i = l;

    for (int j = l; j < r; ++j) {
        if (a[j] <= pivot) {
            int t = a[i];
            a[i] = a[j];
            a[j] = t;
            ++i;
        }
    }

    int t = a[i];
    a[i] = a[r];
    a[r] = t;

    quickSort(a, l, i - 1);
    quickSort(a, i + 1, r);
}

/* BINARY SEARCH - ITERATIVE */

static int binaryIter(const int a[], int n, int key) {
    int l = 0;
    int r = n - 1;

    while (l <= r) {
        int m = l + (r - l) / 2;

        if (a[m] == key) return m;

        if (a[m] < key) {
            l = m + 1;
        } else {
            r = m - 1;
        }
    }

    return -1;
}

/* BINARY SEARCH - RECURSIVE */

static int binaryRec(const int a[],
                     int l, int r, int key) {
    if (l > r) return -1;

    int m = l + (r - l) / 2;

    if (a[m] == key) return m;

    if (key < a[m]) {
        return binaryRec(a, l, m - 1, key);
    }

    return binaryRec(a, m + 1, r, key);
}

/* MAXIMUM SUBARRAY */

static int maxSubarray(const int a[], int n) {
    if (n <= 0) return 0;

    int current = a[0];
    int best = a[0];

    for (int i = 1; i < n; ++i) {
        current = (current < 0) ? a[i] : current + a[i];

        if (current > best) {
            best = current;
        }
    }

    return best;
}

/* FAST EXPONENTIATION */

static long long fastPower(long long base,
                           unsigned int exponent) {
    long long result = 1;

    while (exponent) {
        if (exponent & 1u) {
            result *= base;
        }

        exponent >>= 1;

        if (exponent) {
            base *= base;
        }
    }

    return result;
}

/* CELEBRITY PROBLEM */

static int celebrity(const int knows[][4], int n) {
    if (n <= 0 || n > 4) return -1;

    int candidate = 0;

    for (int i = 1; i < n; ++i) {
        if (knows[candidate][i]) {
            candidate = i;
        }
    }

    for (int i = 0; i < n; ++i) {
        if (i != candidate &&
            (knows[candidate][i] ||
             !knows[i][candidate])) {
            return -1;
        }
    }

    return candidate;
}

/* ACTIVITY SELECTION */
/* Activities must be sorted by finish time. */

static int activitySelection(const int start[],
                             const int end[], int n) {
    int count = 0;
    int lastEnd = INT_MIN;

    for (int i = 0; i < n; ++i) {
        if (start[i] >= lastEnd) {
            ++count;
            lastEnd = end[i];
        }
    }

    return count;
}

/* 0/1 KNAPSACK - DYNAMIC PROGRAMMING */

static int knapsack01(const int wt[], const int val[],
                      int n, int capacity) {
    if (capacity < 0 || n < 0) return -1;

    int *dp = calloc((size_t)capacity + 1, sizeof(int));

    if (!dp) return -1;

    for (int i = 0; i < n; ++i) {
        if (wt[i] <= 0 || val[i] < 0) {
            free(dp);
            return -1;
        }

        for (int w = capacity; w >= wt[i]; --w) {
            int candidate = dp[w - wt[i]] + val[i];

            if (candidate > dp[w]) {
                dp[w] = candidate;
            }
        }
    }

    int result = dp[capacity];

    free(dp);
    return result;
}

/* LONGEST COMMON SUBSEQUENCE */

static int lcs(const char *a, const char *b) {
    size_t n = strlen(a);
    size_t m = strlen(b);

    if (n > 10000 || m > 10000) return -1;

    int *dp = calloc((n + 1) * (m + 1), sizeof(int));

    if (!dp) return -1;

    for (size_t i = 1; i <= n; ++i) {
        for (size_t j = 1; j <= m; ++j) {
            size_t at = i * (m + 1) + j;

            int top = dp[(i - 1) * (m + 1) + j];
            int left = dp[i * (m + 1) + j - 1];

            dp[at] = (a[i - 1] == b[j - 1]) ?
                dp[(i - 1) * (m + 1) + j - 1] + 1 :
                (top > left ? top : left);
        }
    }

    int result = dp[n * (m + 1) + m];

    free(dp);
    return result;
}

/* NAIVE STRING MATCHING */

static int naiveMatch(const char *text,
                      const char *pattern) {
    size_t n = strlen(text);
    size_t m = strlen(pattern);

    for (size_t i = 0; i + m <= n; ++i) {
        if (strncmp(text + i, pattern, m) == 0) {
            return (int)i;
        }
    }

    return -1;
}

/* KMP STRING MATCHING */

static int kmpMatch(const char *text,
                    const char *pattern) {
    size_t m = strlen(pattern);

    if (!m) return 0;

    int *lps = calloc(m, sizeof(int));

    if (!lps) return -1;

    for (size_t i = 1, j = 0; i < m; ++i) {
        while (j > 0 && pattern[i] != pattern[j]) {
            j = (size_t)lps[j - 1];
        }

        if (pattern[i] == pattern[j]) {
            ++j;
        }

        lps[i] = (int)j;
    }

    int found = -1;

    for (size_t i = 0, j = 0; text[i]; ++i) {
        while (j > 0 && text[i] != pattern[j]) {
            j = (size_t)lps[j - 1];
        }

        if (text[i] == pattern[j]) {
            ++j;
        }

        if (j == m) {
            found = (int)(i + 1 - m);
            break;
        }
    }

    free(lps);
    return found;
}

/* INPUT VALIDATION */

static int readInt(const char *prompt, int *value) {
    char line[128];
    char extra;

    printf("%s", prompt);

    if (!fgets(line, sizeof(line), stdin)) {
        return 0;
    }

    return sscanf(line, " %d %c", value, &extra) == 1;
}

/* VIEW PARKING LOCATIONS */

static void viewLots(void) {
    puts("\nID  Parking location          Node  Free/Total  Rs/hr");
    puts("--------------------------------------------------------");

    for (int i = 0; i < lotCount; ++i) {
        printf("%-3d %-25s %-5d %3d/%-6d %d\n",
               lots[i].id,
               lots[i].name,
               lots[i].node,
               lots[i].capacity - lots[i].occupied,
               lots[i].capacity,
               lots[i].price);
    }
}

/* OCCUPANCY REPORT */

static void occupancyReport(void) {
    int cap = 0;
    int occupied = 0;

    for (int i = 0; i < lotCount; ++i) {
        cap += lots[i].capacity;
        occupied += lots[i].occupied;
    }

    printf("\nLocations: %d | Total: %d | Occupied: %d | Available: %d\n",
           lotCount, cap, occupied, cap - occupied);

    printf("Occupancy rate: %.2f%%\n",
           cap ? 100.0 * occupied / cap : 0.0);
}

/* VIEW RESERVATIONS */

static void viewBookings(void) {
    puts("\nReservation  Lot  Driver  Status");
    puts("---------------------------------");

    for (int i = 0; i < bookingCount; ++i) {
        printf("%-12d %-4d %-7d %s\n",
               bookings[i].id,
               bookings[i].lotId,
               bookings[i].driverId,
               bookings[i].active ? "Active" : "Cancelled");
    }

    if (!bookingCount) {
        puts("No reservations recorded.");
    }
}

/* DAA ALGORITHM DEMONSTRATIONS */

static void runWorkbench(void) {
    int a[] = {8, 3, 9, 1, 5, 2};
    int tmp[6];

    mergeSort(a, tmp, 0, 5);

    printf("\nMerge sort: ");

    for (int i = 0; i < 6; ++i) {
        printf("%d ", a[i]);
    }

    printf("\nBinary search for 5: iterative=%d, recursive=%d\n",
           binaryIter(a, 6, 5),
           binaryRec(a, 0, 5, 5));

    int b[] = {8, 3, 9, 1, 5, 2};

    quickSort(b, 0, 5);

    printf("Quick sort: ");

    for (int i = 0; i < 6; ++i) {
        printf("%d ", b[i]);
    }

    int c[] = {-2, 3, -1, 5, -4};

    printf("\nMaximum subarray: %d\n",
           maxSubarray(c, 5));

    printf("Fast power 2^10: %lld\n",
           fastPower(2, 10));

    const int knows[4][4] = {
        {0,1,1,1},
        {0,0,1,1},
        {0,0,0,0},
        {0,0,1,0}
    };

    printf("Celebrity index: %d\n",
           celebrity(knows, 4));

    const int starts[] = {1,3,0,5,8};
    const int ends[] = {2,4,6,7,9};

    printf("Activity selection: %d\n",
           activitySelection(starts, ends, 5));

    const int weights[] = {2,3,4};
    const int values[] = {4,5,7};

    printf("0/1 knapsack: %d\n",
           knapsack01(weights, values, 3, 5));

    printf("LCS(PARKING,KING): %d\n",
           lcs("PARKING", "KING"));

    const char *text = "parking reservation parking";

    printf("Naive matching index: %d\n",
           naiveMatch(text, "reservation"));

    printf("KMP matching index: %d\n",
           kmpMatch(text, "reservation"));
}

/* ALGORITHM BENCHMARK */

static void benchmark(void) {
    int a[MAX_NODES];
    int b[MAX_NODES];
    int same = 1;

    volatile int sink = 0;
    const int repetitions = 10000;

    clock_t start = clock();

    for (int i = 0; i < repetitions; ++i) {
        dijkstra(0, a);
        sink ^= a[i % nodes];
    }

    double dMs =
        1000.0 * (clock() - start) / CLOCKS_PER_SEC;

    start = clock();

    for (int i = 0; i < repetitions; ++i) {
        bellmanFord(0, b);
        sink ^= b[i % nodes];
    }

    double bMs =
        1000.0 * (clock() - start) / CLOCKS_PER_SEC;

    for (int i = 0; i < nodes; ++i) {
        if (a[i] != b[i]) {
            same = 0;
        }
    }

    printf("\nDijkstra: %.3f ms / %d runs\n",
           dMs, repetitions);

    printf("Bellman-Ford: %.3f ms / %d runs\n",
           bMs, repetitions);

    printf("Matching shortest distances: %s\n",
           same ? "YES" : "NO");

    FILE *f = fopen("reports/benchmark.csv", "w");

    if (!f) {
        puts("Could not save reports/benchmark.csv");
        return;
    }

    fprintf(f,
            "algorithm,vertices,runs,total_cpu_ms,matching_distances\n");

    fprintf(f, "Dijkstra,%d,%d,%.6f,%d\n",
            nodes, repetitions, dMs, same);

    fprintf(f, "Bellman-Ford,%d,%d,%.6f,%d\n",
            nodes, repetitions, bMs, same);

    fclose(f);

    (void)sink;

    puts("Saved reports/benchmark.csv");
}

/* SHORTEST ROUTE COMPARISON */

static void routeReport(void) {
    int source;
    int a[MAX_NODES];
    int b[MAX_NODES];

    if (!readInt("Source node (0-5): ", &source) ||
        source < 0 ||
        source >= nodes) {
        puts("Invalid source node.");
        return;
    }

    dijkstra(source, a);
    bellmanFord(source, b);

    puts("Node   Dijkstra   Bellman-Ford");

    for (int i = 0; i < nodes; ++i) {
        printf("%-6d %-10d %d\n",
               i, a[i], b[i]);
    }
}

/* MAIN MENU */

int main(void) {
    initialize();
    loadLots();
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
        puts("0. Exit");

        if (!readInt("Enter choice: ", &choice)) {
            puts("Invalid input or end of input. Exiting.");
            break;
        }

        if (choice == 0) break;

        if (choice == 1) {
            viewLots();
        }
        else if (choice == 2) {
            int source;
            int budget;

            if (!readInt("Driver node (0-5): ", &source) ||
                !readInt("Max price per hour: ", &budget) ||
                source < 0 ||
                source >= nodes ||
                budget < 0) {
                puts("Invalid input.");
                continue;
            }

            int idx = nearest(source, budget, 0);

            if (idx < 0) {
                puts("No feasible parking found.");
            } else {
                printf("Recommended: %s (ID %d, Rs %d/hr)\n",
                       lots[idx].name,
                       lots[idx].id,
                       lots[idx].price);
            }
        }
        else if (choice == 3) {
            int id;
            int driver;
            int idx = -1;

            viewLots();

            if (!readInt("Parking ID: ", &id) ||
                !readInt("Driver ID (positive): ", &driver)) {
                puts("Invalid input.");
                continue;
            }

            for (int i = 0; i < lotCount; ++i) {
                if (lots[i].id == id) {
                    idx = i;
                }
            }

            int reservationId = reserve(idx, driver);

            if (reservationId < 0) {
                puts("Reservation rejected.");
            } else {
                printf("Reservation successful: #%d\n",
                       reservationId);
            }
        }
        else if (choice == 4) {
            int id;

            if (!readInt("Reservation ID: ", &id)) {
                puts("Invalid input.");
            } else {
                puts(cancel(id)
                    ? "Reservation cancelled."
                    : "No active reservation found.");
            }
        }
        else if (choice == 5) {
            occupancyReport();
        }
        else if (choice == 6) {
            viewBookings();
        }
        else if (choice == 7) {
            routeReport();
        }
        else if (choice == 8) {
            runWorkbench();
        }
        else if (choice == 9) {
            benchmark();
        }
        else {
            puts("Unknown menu choice.");
        }
    }

    saveLots();
    saveBookings();

    puts("Goodbye!");

    return 0;
}
 