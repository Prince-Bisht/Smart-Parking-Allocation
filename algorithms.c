#include "algorithms.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

int graph[MAX_NODES][MAX_NODES];
int nodes = 6;

void dijkstra(int source, int dist[MAX_NODES]) {
    int used[MAX_NODES] = {0};
    for (int i = 0; i < nodes; ++i) dist[i] = INF;
    if (source < 0 || source >= nodes) return;
    dist[source] = 0;
    for (int step = 0; step < nodes; ++step) {
        int u = -1;
        for (int i = 0; i < nodes; ++i)
            if (!used[i] && (u == -1 || dist[i] < dist[u])) u = i;
        if (u == -1 || dist[u] == INF) break;
        used[u] = 1;
        for (int v = 0; v < nodes; ++v)
            if (!used[v] && graph[u][v] < INF &&
                dist[v] > dist[u] + graph[u][v])
                dist[v] = dist[u] + graph[u][v];
    }
}

void bellmanFord(int source, int dist[MAX_NODES]) {
    for (int i = 0; i < nodes; ++i) dist[i] = INF;
    if (source < 0 || source >= nodes) return;
    dist[source] = 0;
    for (int step = 1; step < nodes; ++step) {
        int changed = 0;
        for (int u = 0; u < nodes; ++u)
            for (int v = 0; v < nodes; ++v)
                if (dist[u] < INF && graph[u][v] < INF &&
                    dist[v] > dist[u] + graph[u][v]) {
                    dist[v] = dist[u] + graph[u][v];
                    changed = 1;
                }
        if (!changed) break;
    }
}

static void merge(int a[], int tmp[], int l, int m, int r) {
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r)
        tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (i <= m) tmp[k++] = a[i++];
    while (j <= r) tmp[k++] = a[j++];
    for (i = l; i <= r; ++i) a[i] = tmp[i];
}

void mergeSort(int a[], int tmp[], int l, int r) {
    if (l >= r) return;
    int m = l + (r - l) / 2;
    mergeSort(a, tmp, l, m);
    mergeSort(a, tmp, m + 1, r);
    merge(a, tmp, l, m, r);
}

void quickSort(int a[], int l, int r) {
    if (l >= r) return;
    int pivot = a[r], i = l;
    for (int j = l; j < r; ++j) {
        if (a[j] <= pivot) {
            int t = a[i]; a[i] = a[j]; a[j] = t;
            ++i;
        }
    }
    int t = a[i]; a[i] = a[r]; a[r] = t;
    quickSort(a, l, i - 1);
    quickSort(a, i + 1, r);
}

int binaryIter(const int a[], int n, int key) {
    int l = 0, r = n - 1;
    while (l <= r) {
        int m = l + (r - l) / 2;
        if (a[m] == key) return m;
        if (a[m] < key) l = m + 1;
        else r = m - 1;
    }
    return -1;
}

int binaryRec(const int a[], int l, int r, int key) {
    if (l > r) return -1;
    int m = l + (r - l) / 2;
    if (a[m] == key) return m;
    if (key < a[m]) return binaryRec(a, l, m - 1, key);
    return binaryRec(a, m + 1, r, key);
}

int maxSubarray(const int a[], int n) {
    if (n <= 0) return 0;
    int current = a[0], best = a[0];
    for (int i = 1; i < n; ++i) {
        current = (current < 0) ? a[i] : current + a[i];
        if (current > best) best = current;
    }
    return best;
}

long long fastPower(long long base, unsigned int exponent) {
    long long result = 1;
    while (exponent) {
        if (exponent & 1u) result *= base;
        exponent >>= 1;
        if (exponent) base *= base;
    }
    return result;
}

int celebrity(const int knows[][4], int n) {
    if (n <= 0 || n > 4) return -1;
    int candidate = 0;
    for (int i = 1; i < n; ++i)
        if (knows[candidate][i]) candidate = i;
    for (int i = 0; i < n; ++i)
        if (i != candidate && (knows[candidate][i] || !knows[i][candidate]))
            return -1;
    return candidate;
}

/* Requires activities sorted by increasing finish time. */
int activitySelection(const int start[], const int end[], int n) {
    int count = 0, lastEnd = INT_MIN;
    for (int i = 0; i < n; ++i)
        if (start[i] >= lastEnd) { ++count; lastEnd = end[i]; }
    return count;
}

int knapsack01(const int wt[], const int val[], int n, int capacity) {
    if (capacity < 0 || n < 0) return -1;
    int *dp = calloc((size_t)capacity + 1, sizeof(int));
    if (!dp) return -1;
    for (int i = 0; i < n; ++i) {
        if (wt[i] <= 0 || val[i] < 0) { free(dp); return -1; }
        for (int w = capacity; w >= wt[i]; --w) {
            int candidate = dp[w - wt[i]] + val[i];
            if (candidate > dp[w]) dp[w] = candidate;
        }
    }
    int result = dp[capacity];
    free(dp);
    return result;
}

int lcs(const char *a, const char *b) {
    size_t n = strlen(a), m = strlen(b);
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

int naiveMatch(const char *text, const char *pattern) {
    size_t n = strlen(text), m = strlen(pattern);
    for (size_t i = 0; i + m <= n; ++i)
        if (strncmp(text + i, pattern, m) == 0) return (int)i;
    return -1;
}

int kmpMatch(const char *text, const char *pattern) {
    size_t m = strlen(pattern);
    if (!m) return 0;
    int *lps = calloc(m, sizeof(int));
    if (!lps) return -1;
    for (size_t i = 1, j = 0; i < m; ++i) {
        while (j > 0 && pattern[i] != pattern[j]) j = (size_t)lps[j - 1];
        if (pattern[i] == pattern[j]) ++j;
        lps[i] = (int)j;
    }
    int found = -1;
    for (size_t i = 0, j = 0; text[i]; ++i) {
        while (j > 0 && text[i] != pattern[j]) j = (size_t)lps[j - 1];
        if (text[i] == pattern[j]) ++j;
        if (j == m) { found = (int)(i + 1 - m); break; }
    }
    free(lps);
    return found;
}


/* Minimum spanning trees: adjacency matrix with INF meaning no edge. */
int primMST(int n, const int w[MAX_NODES][MAX_NODES]) {
    if (n < 1 || n > MAX_NODES) return -1;
    int key[MAX_NODES], used[MAX_NODES] = {0}, total = 0;
    for (int i = 0; i < n; ++i) key[i] = INF;
    key[0] = 0;
    for (int k = 0; k < n; ++k) {
        int u = -1;
        for (int i = 0; i < n; ++i)
            if (!used[i] && (u < 0 || key[i] < key[u])) u = i;
        if (u < 0 || key[u] == INF) return -1;
        used[u] = 1;
        total += key[u];
        for (int v = 0; v < n; ++v)
            if (!used[v] && u != v && w[u][v] < key[v]) key[v] = w[u][v];
    }
    return total;
}

static int dsuFind(int parent[], int x) {
    if (parent[x] != x) parent[x] = dsuFind(parent, parent[x]);
    return parent[x];
}
int kruskalMST(int n, const int w[MAX_NODES][MAX_NODES]) {
    if (n < 1 || n > MAX_NODES) return -1;
    int parent[MAX_NODES], total = 0, count = 0;
    int used[MAX_NODES][MAX_NODES] = {{0}};
    for (int i = 0; i < n; ++i) parent[i] = i;
    while (count < n-1) {
        int a=-1,b=-1,best=INF;
        for (int i=0;i<n;++i) for(int j=i+1;j<n;++j)
            if(!used[i][j] && w[i][j]<best) {best=w[i][j];a=i;b=j;}
        if(a<0) return -1;
        used[a][b]=1;
        int ra=dsuFind(parent,a),rb=dsuFind(parent,b);
        if(ra!=rb) {parent[ra]=rb;total+=best;++count;}
    }
    return total;
}

void floydWarshall(int n, const int w[MAX_NODES][MAX_NODES],
                   int out[MAX_NODES][MAX_NODES]) {
    if (n<0 || n>MAX_NODES) return;
    for(int i=0;i<n;++i) for(int j=0;j<n;++j) out[i][j]=w[i][j];
    for(int k=0;k<n;++k) for(int i=0;i<n;++i) for(int j=0;j<n;++j)
        if(out[i][k]<INF && out[k][j]<INF &&
           out[i][j]>out[i][k]+out[k][j])
            out[i][j]=out[i][k]+out[k][j];
}

/* Edmonds-Karp: BFS augmenting-path variant of Ford-Fulkerson. */
int maxFlow(int n, const int capacity[MAX_NODES][MAX_NODES], int source, int sink) {
    if(n<1 || n>MAX_NODES || source<0 || source>=n || sink<0 || sink>=n || source==sink) return -1;
    int residual[MAX_NODES][MAX_NODES], flow=0;
    for(int i=0;i<n;++i) for(int j=0;j<n;++j) {
        if(capacity[i][j]<0) return -1;
        residual[i][j]=capacity[i][j];
    }
    while(1) {
        int parent[MAX_NODES],queue[MAX_NODES],head=0,tail=0;
        for(int i=0;i<n;++i) parent[i]=-1;
        parent[source]=source;queue[tail++]=source;
        while(head<tail && parent[sink]<0) {
            int u=queue[head++];
            for(int v=0;v<n;++v) if(parent[v]<0 && residual[u][v]>0) {
                parent[v]=u;queue[tail++]=v;
            }
        }
        if(parent[sink]<0) break;
        int aug=INT_MAX;
        for(int v=sink;v!=source;v=parent[v])
            if(residual[parent[v]][v]<aug) aug=residual[parent[v]][v];
        for(int v=sink;v!=source;v=parent[v]) {
            residual[parent[v]][v]-=aug;
            residual[v][parent[v]]+=aug;
        }
        flow+=aug;
    }
    return flow;
}

static int matchDfs(int u,int right,const int edges[MAX_NODES][MAX_NODES],
                    int match[],int seen[]) {
    for(int v=0;v<right;++v) if(edges[u][v] && !seen[v]) {
        seen[v]=1;
        if(match[v]<0 || matchDfs(match[v],right,edges,match,seen)) {
            match[v]=u;return 1;
        }
    }
    return 0;
}
int bipartiteMatching(int left,int right,const int edges[MAX_NODES][MAX_NODES]) {
    if(left<0 || right<0 || left>MAX_NODES || right>MAX_NODES) return -1;
    int match[MAX_NODES],total=0;
    for(int v=0;v<right;++v) match[v]=-1;
    for(int u=0;u<left;++u) {
        int seen[MAX_NODES]={0};total+=matchDfs(u,right,edges,match,seen);
    }
    return total;
}

static long long extendedGcd(long long a,long long b,long long *x,long long *y) {
    if(!b) {*x=1;*y=0;return a;}
    long long x1,y1,g=extendedGcd(b,a%b,&x1,&y1);
    *x=y1;*y=x1-(a/b)*y1;return g;
}
long long chineseRemainder(const int remainder[],const int moduli[],int n) {
    if(n<1 || n>12) return -1;
    long long result=0,product=1;
    for(int i=0;i<n;++i) {
        if(moduli[i]<=1 || product>LLONG_MAX/moduli[i]) return -1;
        product*=moduli[i];
    }
    for(int i=0;i<n;++i) {
        long long part=product/moduli[i],x,y;
        if(extendedGcd(part,moduli[i],&x,&y)!=1) return -1;
        long long inv=(x%moduli[i]+moduli[i])%moduli[i];
        long long r=((long long)remainder[i]%moduli[i]+moduli[i])%moduli[i];
        /* Modular multiplication avoids overflow for moderate demonstration inputs. */
        long long term=0,add=part%product;
        long long times=(r*inv)%moduli[i];
        while(times) {
            if(times&1) term=(term+add)%product;
            add=(add+add)%product;times>>=1;
        }
        result=(result+term)%product;
    }
    return result;
}

int rabinKarp(const char *text,const char *pattern) {
    int n=(int)strlen(text),m=(int)strlen(pattern);
    if(!m) return 0;
    if(m>n) return -1;
    const long long mod=1000000007LL,base=256;
    long long h=1,ph=0,th=0;
    for(int i=1;i<m;++i) h=h*base%mod;
    for(int i=0;i<m;++i) {
        ph=(ph*base+(unsigned char)pattern[i])%mod;
        th=(th*base+(unsigned char)text[i])%mod;
    }
    for(int i=0;i<=n-m;++i) {
        if(ph==th && memcmp(text+i,pattern,(size_t)m)==0) return i;
        if(i<n-m) {
            th=(th-(unsigned char)text[i]*h%mod+mod)%mod;
            th=(th*base+(unsigned char)text[i+m])%mod;
        }
    }
    return -1;
}

/* DFA transition table built for the pattern, alphabet = 256 byte values. */
int finiteAutomatonMatch(const char *text,const char *pattern) {
    int m=(int)strlen(pattern);
    if(m==0) return 0;
    if(m>1024) return -1;
    int *table=calloc((size_t)(m+1)*256,sizeof(int));
    if(!table) return -1;
    for(int state=0;state<=m;++state) for(int ch=0;ch<256;++ch) {
        int limit=state+1;
        if(limit>m) limit=m;
        for(int k=limit;k>0;--k) {
            if((unsigned char)pattern[k-1]!=(unsigned char)ch) continue;
            int ok=1;
            for(int j=0;j<k-1;++j) {
                int index=state-(k-1)+j;
                if(index<0 || pattern[j]!=pattern[index]) {ok=0;break;}
            }
            if(ok) {table[state*256+ch]=k;break;}
        }
    }
    int state=0,found=-1;
    for(int i=0;text[i];++i) {
        state=table[state*256+(unsigned char)text[i]];
        if(state==m) {found=i-m+1;break;}
    }
    free(table);return found;
}

static void queenDfs(int row,int n,unsigned columns,unsigned diag1,unsigned diag2,int *total) {
    if(row==n) {++*total;return;}
    unsigned all=(1u<<n)-1u;
    unsigned freePos=all&~(columns|diag1|diag2);
    while(freePos) {
        unsigned bit=freePos&(~freePos+1u);freePos-=bit;
        queenDfs(row+1,n,columns|bit,((diag1|bit)<<1)&all,
                 (diag2|bit)>>1,total);
    }
}
int nQueensCount(int n) {
    if(n<1 || n>12) return -1;
    int total=0;queenDfs(0,n,0,0,0,&total);return total;
}

static int subsetDfs(const int a[],int n,int i,int target) {
    if(target==0) return 1;
    if(i==n || target<0) return 0;
    return subsetDfs(a,n,i+1,target-a[i]) || subsetDfs(a,n,i+1,target);
}
int subsetSumExists(const int a[],int n,int target) {
    if(n<0 || n>30 || target<0) return -1;
    for(int i=0;i<n;++i) if(a[i]<0) return -1;
    return subsetDfs(a,n,0,target);
}

int matrixChainCost(const int dims[],int count) {
    if(count<2 || count>32) return -1;
    int dp[32][32]={{0}};
    for(int i=0;i<count;++i) if(dims[i]<=0) return -1;
    int n=count-1;
    for(int len=2;len<=n;++len) for(int i=0;i+len<=n;++i) {
        int j=i+len-1;dp[i][j]=INF;
        for(int k=i;k<j;++k) {
            long long cost=(long long)dp[i][k]+dp[k+1][j]+(long long)dims[i]*dims[k+1]*dims[j+1];
            if(cost<dp[i][j]) dp[i][j]=(int)cost;
        }
    }
    return dp[0][n-1];
}
int optimalBSTCost(const int freq[],int n) {
    if(n<0 || n>32) return -1;
    if(!n) return 0;
    int dp[32][32]={{0}},prefix[33]={0};
    for(int i=0;i<n;++i) {
        if(freq[i]<0) return -1;
        prefix[i+1]=prefix[i]+freq[i];
        dp[i][i]=freq[i];
    }
    for(int len=2;len<=n;++len) for(int i=0;i+len<=n;++i) {
        int j=i+len-1;dp[i][j]=INF;
        for(int root=i;root<=j;++root) {
            int cost=(root>i?dp[i][root-1]:0)+(root<j?dp[root+1][j]:0);
            if(cost<dp[i][j]) dp[i][j]=cost;
        }
        dp[i][j]+=prefix[j+1]-prefix[i];
    }
    return dp[0][n-1];
}
int tspHeldKarp(int n,const int cost[MAX_NODES][MAX_NODES]) {
    if(n<2 || n>15) return -1;
    int masks=1<<n;
    int *dp=malloc((size_t)masks*n*sizeof(int));
    if(!dp) return -1;
    for(int i=0;i<masks*n;++i) dp[i]=INF;
    dp[n]=0; /* state mask=1, ending at city 0 */
    for(int mask=1;mask<masks;mask+=2) for(int u=0;u<n;++u) {
        int current=dp[mask*n+u];
        if(current>=INF) continue;
        for(int v=1;v<n;++v) if(!(mask&(1<<v)) && cost[u][v]<INF) {
            int next=mask|(1<<v);
            int candidate=current+cost[u][v];
            if(candidate<dp[next*n+v]) dp[next*n+v]=candidate;
        }
    }
    int best=INF;
    for(int u=1;u<n;++u) if(dp[(masks-1)*n+u]<INF && cost[u][0]<INF) {
        int candidate=dp[(masks-1)*n+u]+cost[u][0];
        if(candidate<best) best=candidate;
    }
    free(dp);return best==INF?-1:best;
}

double fractionalKnapsack(const int wt[],const int val[],int n,int capacity) {
    if(n<0 || n>128 || capacity<0) return -1;
    int used[128]={0};double result=0;
    for(int k=0;k<n && capacity>0;++k) {
        int best=-1;
        for(int i=0;i<n;++i) {
            if(wt[i]<=0 || val[i]<0) return -1;
            if(!used[i] && (best<0 || (double)val[i]/wt[i]>(double)val[best]/wt[best])) best=i;
        }
        if(best<0) break;
        used[best]=1;
        int take=wt[best]<capacity?wt[best]:capacity;
        result+=(double)val[best]*take/wt[best];capacity-=take;
    }
    return result;
}
int intervalPartitionCount(const int start[],const int end[],int n) {
    if(n<0 || n>128) return -1;
    int used[128]={0},roomsEnd[128],rooms=0;
    for(int k=0;k<n;++k) {
        int next=-1;
        for(int i=0;i<n;++i) if(!used[i] && (next<0 || start[i]<start[next])) next=i;
        if(next<0 || end[next]<start[next]) return -1;
        used[next]=1;
        int room=0;
        while(room<rooms && roomsEnd[room]>start[next]) ++room;
        if(room==rooms) ++rooms;
        roomsEnd[room]=end[next];
    }
    return rooms;
}
