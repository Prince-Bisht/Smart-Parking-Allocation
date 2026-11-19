/* SmartParking - C17 / raylib, single source file. */
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#define MAX_LOTS 32
#define MAX_RES 256
#define MAX_NODES 32
#define INF 100000000

typedef struct {int id,node,capacity,occupied,price; char name[48];} Lot;
typedef struct {int id,lotId,driverId,active;} Reservation;
static Lot lots[MAX_LOTS]; static int lotCount=0;
static Reservation bookings[MAX_RES]; static int bookingCount=0, nextBooking=1;
static int graph[MAX_NODES][MAX_NODES],nodes=6;
static char status[180]="Ready";
static int screen=0, selected=0, driverNode=0, driverId=101, budget=60;
static Color bg={12,20,36,255}, panel={25,37,57,255}, accent={56,204,154,255};
static void message(const char *s){snprintf(status,sizeof(status),"%s",s);}
static void defaults(void){
 const Lot seed[]={{1,0,8,2,25,"Clock Tower"},{2,1,6,1,35,"Rajpur Road"},{3,2,10,4,20,"ISBT"},{4,3,5,0,40,"Prem Nagar"},{5,4,7,3,30,"Ballupur"},{6,5,9,1,15,"Sahastradhara"}};
 memcpy(lots,seed,sizeof(seed));lotCount=6;bookingCount=0;nextBooking=1;
 for(int i=0;i<MAX_NODES;i++)for(int j=0;j<MAX_NODES;j++)graph[i][j]=i==j?0:INF;
 int edges[][3]={{0,1,4},{0,2,7},{0,4,3},{1,3,5},{1,5,8},{2,3,4},{2,4,2},{3,5,3},{4,5,6}};
 for(size_t i=0;i<sizeof(edges)/sizeof(edges[0]);i++){int a=edges[i][0],b=edges[i][1],w=edges[i][2];graph[a][b]=graph[b][a]=w;}
}
static int loadLots(void){FILE*f=fopen("data/parking.csv","r");if(!f)return 0;char line[256];if(!fgets(line,sizeof line,f)){fclose(f);return 0;}Lot temp[MAX_LOTS];int count=0;
 while(count<MAX_LOTS&&fgets(line,sizeof line,f)){Lot x; if(sscanf(line,"%d,%d,%d,%d,%d,%47[^\r\n]",&x.id,&x.node,&x.capacity,&x.occupied,&x.price,x.name)==6 && x.id>0 && x.node>=0&&x.node<nodes&&x.capacity>=0&&x.occupied>=0&&x.occupied<=x.capacity&&x.price>=0){temp[count++]=x;}}
 fclose(f);if(!count)return 0;memcpy(lots,temp,(size_t)count*sizeof(Lot));lotCount=count;return 1;}
static void saveLots(void){FILE*f=fopen("data/parking.csv","w");if(!f){message("Cannot write data/parking.csv");return;}fprintf(f,"id,node,capacity,occupied,price,name\n");for(int i=0;i<lotCount;i++)fprintf(f,"%d,%d,%d,%d,%d,%s\n",lots[i].id,lots[i].node,lots[i].capacity,lots[i].occupied,lots[i].price,lots[i].name);fclose(f);}
static void saveBookings(void){FILE*f=fopen("data/reservations.csv","w");if(!f)return;fprintf(f,"reservation_id,lot_id,driver_id,active\n");for(int i=0;i<bookingCount;i++)fprintf(f,"%d,%d,%d,%d\n",bookings[i].id,bookings[i].lotId,bookings[i].driverId,bookings[i].active);fclose(f);}
/* Dijkstra: non-negative weights. O(V^2), O(V) extra memory. */
static void dijkstra(int source,int dist[MAX_NODES]){int used[MAX_NODES]={0};for(int i=0;i<nodes;i++)dist[i]=INF;dist[source]=0;for(int k=0;k<nodes;k++){int u=-1;for(int i=0;i<nodes;i++)if(!used[i]&&(u<0||dist[i]<dist[u]))u=i;if(u<0||dist[u]==INF)break;used[u]=1;for(int v=0;v<nodes;v++)if(graph[u][v]<INF&&dist[v]>dist[u]+graph[u][v])dist[v]=dist[u]+graph[u][v];}}
/* Bellman-Ford, alternative shortest path solver. */
static void bellmanFord(int source,int dist[MAX_NODES]){for(int i=0;i<nodes;i++)dist[i]=INF;dist[source]=0;for(int k=1;k<nodes;k++){int changed=0;for(int u=0;u<nodes;u++)for(int v=0;v<nodes;v++)if(dist[u]<INF&&graph[u][v]<INF&&dist[v]>dist[u]+graph[u][v]){dist[v]=dist[u]+graph[u][v];changed=1;}if(!changed)break;}}
static int nearest(int source,int maxPrice,int bellman){int dist[MAX_NODES];if(bellman)bellmanFord(source,dist);else dijkstra(source,dist);int best=-1;for(int i=0;i<lotCount;i++)if(lots[i].occupied<lots[i].capacity&&lots[i].price<=maxPrice&&dist[lots[i].node]<INF){if(best<0||dist[lots[i].node]<dist[lots[best].node]||(dist[lots[i].node]==dist[lots[best].node]&&lots[i].price<lots[best].price))best=i;}return best;}
static int reserve(int idx,int driver){if(idx<0||idx>=lotCount||lots[idx].occupied>=lots[idx].capacity||bookingCount>=MAX_RES)return -1;int id=nextBooking++;bookings[bookingCount++]=(Reservation){id,lots[idx].id,driver,1};lots[idx].occupied++;saveLots();saveBookings();return id;}
static int cancelLatest(void){for(int i=bookingCount-1;i>=0;i--)if(bookings[i].active){bookings[i].active=0;for(int j=0;j<lotCount;j++)if(lots[j].id==bookings[i].lotId&&lots[j].occupied>0){lots[j].occupied--;break;}saveLots();saveBookings();return bookings[i].id;}return -1;}
/* Student-written sorting and searching implementations. */
static void merge(int a[],int tmp[],int l,int m,int r){int i=l,j=m+1,k=l;while(i<=m&&j<=r)tmp[k++]=a[i]<=a[j]?a[i++]:a[j++];while(i<=m)tmp[k++]=a[i++];while(j<=r)tmp[k++]=a[j++];for(i=l;i<=r;i++)a[i]=tmp[i];}
static void mergeSort(int a[],int tmp[],int l,int r){if(l>=r)return;int m=l+(r-l)/2;mergeSort(a,tmp,l,m);mergeSort(a,tmp,m+1,r);merge(a,tmp,l,m,r);}
static void quickSort(int a[],int l,int r){if(l>=r)return;int pivot=a[r],i=l;for(int j=l;j<r;j++)if(a[j]<=pivot){int t=a[i];a[i]=a[j];a[j]=t;i++;}int t=a[i];a[i]=a[r];a[r]=t;quickSort(a,l,i-1);quickSort(a,i+1,r);}
static int binaryIter(const int a[],int n,int key){int l=0,r=n-1;while(l<=r){int m=l+(r-l)/2;if(a[m]==key)return m;if(a[m]<key)l=m+1;else r=m-1;}return -1;}
static int binaryRec(const int a[],int l,int r,int key){if(l>r)return -1;int m=l+(r-l)/2;if(a[m]==key)return m;return key<a[m]?binaryRec(a,l,m-1,key):binaryRec(a,m+1,r,key);}
static int maxSubarray(const int a[],int n){int best=INT_MIN,sum=0;for(int i=0;i<n;i++){sum=(sum>0?sum:0)+a[i];if(sum>best)best=sum;}return best;}
static long long fastPower(long long b,unsigned int e){long long result=1;while(e){if(e&1u)result*=b;b*=b;e>>=1;}return result;}
static int celebrity(const int knows[][4],int n){int candidate=0;for(int i=1;i<n;i++)if(knows[candidate][i])candidate=i;for(int i=0;i<n;i++)if(i!=candidate&&(knows[candidate][i]||!knows[i][candidate]))return -1;return candidate;}
static int activitySelection(const int start[],const int end[],int n){int count=0,last=INT_MIN;for(int i=0;i<n;i++)if(start[i]>=last){count++;last=end[i];}return count;} /* assumes sorted by finish time */
static int knapsack01(const int wt[],const int val[],int n,int cap){int *dp=calloc((size_t)cap+1,sizeof(int));if(!dp)return -1;for(int i=0;i<n;i++)for(int w=cap;w>=wt[i];w--){int candidate=dp[w-wt[i]]+val[i];if(candidate>dp[w])dp[w]=candidate;}int result=dp[cap];free(dp);return result;}
static int lcs(const char*a,const char*b){int n=(int)strlen(a),m=(int)strlen(b);int *dp=calloc((size_t)(n+1)*(m+1),sizeof(int));if(!dp)return -1;for(int i=1;i<=n;i++)for(int j=1;j<=m;j++){int at=i*(m+1)+j;dp[at]=a[i-1]==b[j-1]?dp[(i-1)*(m+1)+j-1]+1:(dp[(i-1)*(m+1)+j]>dp[i*(m+1)+j-1]?dp[(i-1)*(m+1)+j]:dp[i*(m+1)+j-1]);}int result=dp[n*(m+1)+m];free(dp);return result;}
static int naiveMatch(const char*t,const char*p){int n=(int)strlen(t),m=(int)strlen(p);for(int i=0;i+m<=n;i++)if(strncmp(t+i,p,(size_t)m)==0)return i;return -1;}
static int kmpMatch(const char*t,const char*p){int m=(int)strlen(p);if(!m)return 0;int *pi=calloc((size_t)m,sizeof(int));if(!pi)return -1;for(int i=1,j=0;i<m;i++){while(j&&p[i]!=p[j])j=pi[j-1];if(p[i]==p[j])j++;pi[i]=j;}int found=-1;for(int i=0,j=0;t[i];i++){while(j&&t[i]!=p[j])j=pi[j-1];if(t[i]==p[j])j++;if(j==m){found=i-m+1;break;}}free(pi);return found;}
static void benchmark(void){FILE*f=fopen("reports/benchmark.csv","w");if(!f){message("Cannot write reports/benchmark.csv");return;}fprintf(f,"algorithm,vertices,elapsed_ms,matching_distances\n");int a[MAX_NODES],b[MAX_NODES];clock_t s=clock();for(int k=0;k<10000;k++)dijkstra(0,a);double t1=1000.0*(clock()-s)/CLOCKS_PER_SEC;s=clock();for(int k=0;k<10000;k++)bellmanFord(0,b);double t2=1000.0*(clock()-s)/CLOCKS_PER_SEC;int same=1;for(int i=0;i<nodes;i++)if(a[i]!=b[i])same=0;fprintf(f,"Dijkstra,%d,%.3f,%d\n",nodes,t1,same);fprintf(f,"Bellman-Ford,%d,%.3f,%d\n",nodes,t2,same);fclose(f);message("Benchmark saved (10,000 runs each)");}
static int button(Rectangle r,const char*label){Vector2 m=GetMousePosition();int hover=CheckCollisionPointRec(m,r);DrawRectangleRounded(r,0.18f,8,hover?(Color){58,88,124,255}:panel);DrawRectangleRoundedLinesEx(r,0.18f,8,1,hover?accent:(Color){67,83,110,255});DrawText(label,(int)r.x+13,(int)r.y+11,18,RAYWHITE);return hover&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT);}
static void labelValue(int x,int y,const char*name,int value){DrawText(name,x,y,17,LIGHTGRAY);DrawText(TextFormat("%d",value),x,y+26,29,accent);}
static void dashboard(void){int cap=0,occ=0;for(int i=0;i<lotCount;i++){cap+=lots[i].capacity;occ+=lots[i].occupied;}labelValue(35,115,"TOTAL SLOTS",cap);labelValue(265,115,"AVAILABLE",cap-occ);labelValue(495,115,"OCCUPIED",occ);DrawText("Parking locations (click to select)",35,213,22,RAYWHITE);
 for(int i=0;i<lotCount;i++){int x=35+(i%3)*270,y=255+(i/3)*110;Rectangle r={(float)x,(float)y,250,90};DrawRectangleRounded(r,0.13f,8,i==selected?(Color){42,85,91,255}:panel);DrawText(lots[i].name,x+12,y+10,18,RAYWHITE);DrawText(TextFormat("Free %d/%d  |  Rs %d/hr",lots[i].capacity-lots[i].occupied,lots[i].capacity,lots[i].price),x+12,y+47,16,LIGHTGRAY);if(CheckCollisionPointRec(GetMousePosition(),r)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))selected=i;}
 if(button((Rectangle){35,510,210,45},"Reserve selected")){int id=reserve(selected,driverId);message(id<0?"Cannot reserve: full or limit reached":TextFormat("Reservation #%d created",id));}
 if(button((Rectangle){260,510,220,45},"Cancel latest")){int id=cancelLatest();message(id<0?"No active reservation":TextFormat("Cancelled reservation #%d",id));}
 if(button((Rectangle){495,510,210,45},"Find nearest")){int idx=nearest(driverNode,budget,0);if(idx<0)message("No feasible parking found");else{selected=idx;message(TextFormat("Recommended: %s",lots[idx].name));}}
}
static void algorithms(void){DrawText("Algorithm Workbench",35,115,29,RAYWHITE);DrawText("Algorithms below are implemented in this single C source.",35,155,18,LIGHTGRAY);
 if(button((Rectangle){35,205,240,45},"Sort / Binary Search")){int a[]={8,3,9,1,5,2},tmp[6];mergeSort(a,tmp,0,5);int pos=binaryIter(a,6,5);quickSort(a,0,5);message(TextFormat("Merge/Quick sorted; binary 5 at index %d (rec %d)",pos,binaryRec(a,0,5,5)));}
 if(button((Rectangle){295,205,240,45},"Max Subarray / Power")){int a[]={-2,3,-1,5,-4};message(TextFormat("Max subarray=%d, 2^10=%lld",maxSubarray(a,5),fastPower(2,10)));}
 if(button((Rectangle){555,205,240,45},"DP Knapsack / LCS")){int w[]={2,3,4},v[]={4,5,7};message(TextFormat("Knapsack=%d, LCS=%d",knapsack01(w,v,3,5),lcs("PARKING","KING")));}
 if(button((Rectangle){35,270,240,45},"Activity Selection")){int s[]={1,3,0,5,8},e[]={2,4,6,7,9};message(TextFormat("Selected activities: %d",activitySelection(s,e,5)));}
 if(button((Rectangle){295,270,240,45},"Naive / KMP")){const char*t="parking reservation parking";message(TextFormat("Naive index=%d, KMP index=%d",naiveMatch(t,"reservation"),kmpMatch(t,"reservation")));}
 if(button((Rectangle){555,270,240,45},"Celebrity")){const int k[4][4]={{0,1,1,1},{0,0,1,1},{0,0,0,0},{0,0,1,0}};message(TextFormat("Celebrity index=%d",celebrity(k,4)));}
 if(button((Rectangle){35,350,270,45},"Run route benchmark"))benchmark();
 DrawText("Routing: Dijkstra vs Bellman-Ford",35,430,21,accent);DrawText("Benchmarks write to reports/benchmark.csv",35,463,17,LIGHTGRAY);
}
int main(void){defaults();loadLots();InitWindow(850,650,"SmartParking - C17 / raylib");SetTargetFPS(60);while(!WindowShouldClose()){BeginDrawing();ClearBackground(bg);DrawText("SMARTPARK",35,25,34,accent);DrawText("Urban Parking Allocation Simulator",35,69,18,LIGHTGRAY);if(button((Rectangle){560,26,120,43},"Dashboard"))screen=0;if(button((Rectangle){695,26,120,43},"DAA Lab"))screen=1;if(screen==0)dashboard();else algorithms();DrawRectangle(0,595,850,55,panel);DrawText(status,25,612,16,RAYWHITE);EndDrawing();}saveLots();saveBookings();CloseWindow();return 0;}
