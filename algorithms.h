#ifndef ALGORITHMS_H
#define ALGORITHMS_H
#define MAX_NODES 32
#define INF 100000000
extern int graph[MAX_NODES][MAX_NODES];
extern int nodes;
void dijkstra(int source,int dist[MAX_NODES]);
void bellmanFord(int source,int dist[MAX_NODES]);
void mergeSort(int a[],int tmp[],int l,int r);
void quickSort(int a[],int l,int r);
int binaryIter(const int a[],int n,int key);
int binaryRec(const int a[],int l,int r,int key);
int maxSubarray(const int a[],int n);
long long fastPower(long long base,unsigned int exponent);
int celebrity(const int knows[][4],int n);
int activitySelection(const int start[],const int end[],int n);
double fractionalKnapsack(const int wt[],const int val[],int n,int capacity);
int intervalPartitionCount(const int start[],const int end[],int n);
int knapsack01(const int wt[],const int val[],int n,int capacity);
int lcs(const char *a,const char *b);
int matrixChainCost(const int dims[],int count);
int optimalBSTCost(const int freq[],int n);
int tspHeldKarp(int n,const int cost[MAX_NODES][MAX_NODES]);
int primMST(int n,const int w[MAX_NODES][MAX_NODES]);
int kruskalMST(int n,const int w[MAX_NODES][MAX_NODES]);
void floydWarshall(int n,const int w[MAX_NODES][MAX_NODES],int out[MAX_NODES][MAX_NODES]);
int maxFlow(int n,const int capacity[MAX_NODES][MAX_NODES],int source,int sink);
int bipartiteMatching(int left,int right,const int edges[MAX_NODES][MAX_NODES]);
int nQueensCount(int n);
int subsetSumExists(const int a[],int n,int target);
long long chineseRemainder(const int remainder[],const int moduli[],int n);
int naiveMatch(const char *text,const char *pattern);
int kmpMatch(const char *text,const char *pattern);
int rabinKarp(const char *text,const char *pattern);
int finiteAutomatonMatch(const char *text,const char *pattern);
#endif
