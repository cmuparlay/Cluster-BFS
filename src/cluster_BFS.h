#include <atomic>
#include <utility>
#include <unordered_map>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/io.h>

#include "ligra_light.h"
#include "BFS_ligra.h"

template <typename vertex,typename graph, typename distance, typename label, int R>
void cluster_BFS(parlay::sequence<vertex>& vertices, const graph& G, parlay::sequence<std::array<label,R>>& S, parlay::sequence<distance>& D){
  parlay::internal::timer t("cluster_BFS",false);
  size_t n =G.size();
  const distance INF=(1<<sizeof(distance)*8-1)-1;
  parlay::sequence<std::atomic<label>> S0(n);
  parlay::sequence<std::atomic<label>> S1(n);
  parlay::sequence<std::atomic<distance>> distances(n);
  distance round = 0;
  parlay::parallel_for(0,n, [&](vertex i){
    S0[i]=0;S1[i]=0;D[i]=INF;distances[i]=INF;
    for (int j = 0;j<R; j++){S[i][j]=0;}
  });
  parlay::sequence<vertex> seeds;
  for (size_t i = 0; i<vertices.size(); i++){
    vertex v = vertices[i];
    if (i != 0 && v==vertices[0]){break;}
    S1[v] = 1ul <<i;
    seeds.push_back(v);
  }
  t.next("init");
  // printf("seeds: ");
  // for (auto v:seeds){
  //   printf("%d ", v);
  // }
  // printf("\n");
  
  auto edge_f = [&] (vertex u, vertex v) -> bool {
    bool success= false;
    label u_visited = S0[u].load();
    label v_visited = S1[v].load();
    if ((u_visited | v_visited) != v_visited) {
      S1[v].fetch_or(u_visited);
      distance old_d = distances[v].load();
      if(old_d != round && distances[v].compare_exchange_strong(old_d, round))
        success=true;
    };
    return success;
  };
  auto frontier_f = [&](vertex v){
    label difference = S1[v].load() &~ S0[v].load();
    if (D[v]==INF){D[v]=round;}
    S[v][round-D[v]]=difference;
    S0[v]|=difference;
  };
  auto cond_f = [&] (vertex v) {return D[v]==INF || round-D[v]<R;};
  auto frontier_map = ligra::edge_map(G, G, edge_f, cond_f);
  auto frontier = ligra::vertex_subset<vertex>();
  frontier.add_vertices(seeds);
  t.next("head");

  long total = 0;
  vertex v = 99;
  while (frontier.size() > 0) {
    frontier.apply(frontier_f);
    round++;
    long m = frontier.size();
    total += m;
    frontier = frontier_map(frontier, false);
    // t.next("map");
    t.next("update");
  }
}


template <typename vertex,typename graph, typename distance, typename label, int R>
void verify_CBFS(parlay::sequence<vertex>& vertices, const graph& G, parlay::sequence<std::array<label,R>>& S, parlay::sequence<distance>& D){
  const distance INF=(1<<sizeof(distance)*8-1)-1;
  size_t n = G.size();
  parlay::sequence<distance> answer(n);
  for (size_t j = 0; j<vertices.size(); j++){
    if (j!=0 && vertices[j]==vertices[0]){break;}
    BFS(vertices[j], G, G, answer);
    for (size_t v = 0; v<n; v++){
      distance d_true = answer[v];
      distance d_query=D[v];
      if (d_true == INF){continue;}
      label sum = 0;
      bool changed=false;
      for (size_t r = 0;r<R; r++){
        sum|=S[v][r];
        if (sum&(1ul<<j)){
          d_query += r;
          changed=true;
          break;
        }
      }
      
      if (changed && d_query!=d_true){
        printf("source: %d, vertex_id %d, target: %d, d_true: %d, d_query: %d, D: %d\n",vertices[j], j,v,d_true, d_query, D[v]);
        for (int i = 0; i<R; i++){
          printf("S[%d]: %lu\n", i, S[v][i]);
        }
        return;
      }else if (!changed && d_true-d_query>((R+1)/2)*2){
        printf("source: %d, vertex_id %d, target: %d, d_true: %u, d_query: %u out of range\n", vertices[j],j,v,d_true, d_query);
        return;
      }
    }
  }
  printf("PASS\n");
}