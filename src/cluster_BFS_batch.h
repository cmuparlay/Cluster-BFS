#include <atomic>
#include <utility>
#include <unordered_map>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/io.h>

#include "ligra_light.h"
#include "BFS_ligra.h"

template <typename vertex,typename graph, typename distance, typename label, int R>
void cluster_BFS_batch(parlay::sequence<parlay::sequence<vertex>>& vertices, const graph& G, parlay::sequence<std::array<label,R>>& S, parlay::sequence<distance>& D){
  parlay::internal::timer t("k_BFS_batch",false);
  size_t n =G.size();
  const distance INF=(1<<sizeof(distance)*8-1)-1;
  size_t num_batches = vertices.size();
  size_t label_size = num_batches*n;
  parlay::sequence<std::atomic<label>> S0(label_size);
  parlay::sequence<std::atomic<label>> S1(label_size);
  parlay::sequence<std::atomic<distance>> distances(n);
  distance round = 0;
  parlay::parallel_for(0,label_size, [&](size_t i){
    S0[i]=0;S1[i]=0;D[i]=INF; 
  });
  parlay::parallel_for(0,n,[&](vertex i){distances[i]=INF;}); 
  
  parlay::sequence<vertex> seeds;
  for (size_t i = 0; i<num_batches; i++){
    for (size_t j = 0; j<vertices[i].size(); j++){
      if (j!=0 && vertices[i][j]==vertices[i][0]){break;}
      size_t v = vertices[i][j];
      S1[(v*num_batches)+i]=1ul<<j;
      seeds.push_back(v);
    }
  }
  t.next("init");

  
  auto edge_f = [&] (vertex u, vertex v) -> bool {
    bool success= false;
    size_t index_u=u*num_batches;
    size_t index_v=v*num_batches;
    for (size_t i = 0; i<num_batches; i++){
      if (D[index_v+i]==INF||round - D[index_v+i]<R){
		  label u_visited = S0[index_u+i].load();
		  label v_visited = S1[index_v+i].load();
		  if ((u_visited | v_visited) != v_visited) {
		    S1[index_v+i].fetch_or(u_visited);
		    distance old_d = distances[v].load();
		    if(old_d != round && distances[v].compare_exchange_strong(old_d, round))
          success=true;
		  };
      }
    }
    return success;
    };

  auto cond_f = [&] (vertex v) {return true;};
  auto frontier_f = [&](vertex v){
    size_t index_base = (v*num_batches);
    for (size_t i = 0; i<num_batches; i++){
      size_t index_v= index_base+i;
      // if (visit_first[visit_index]== INF || round - visit_first[visit_index]<R){
        label newly_visited = S1[index_v].load() &~ (S0[index_v].load());
        // for bit newly visited, set v's distances to round
        if (newly_visited!=0){
          if (D[index_v]==INF) D[index_v]=round;
          distance index = round - D[index_v];
          S[index_v][index]=newly_visited;
          S0[index_v] |= newly_visited;
        }
      // }
    }
  };
  auto frontier_map = ligra::edge_map(G, G, edge_f, cond_f);
  auto frontier = ligra::vertex_subset<vertex>();
  frontier.add_vertices(seeds);
  t.next("head");

  long total = 0;
  while (frontier.size() > 0) {
    frontier.apply(frontier_f);
    round++;
    long m = frontier.size();
    total += m;
    frontier = frontier_map(frontier, false);
    t.next("map");
    t.next("update");
  }
  parlay::parallel_for(0, D.size(),[&](size_t i){
    if (D[i]!=INF){
      D[i]+=(R-1);
    }
  });
  // nodes[vertices[0]].s0=0;
}


template <typename vertex,typename graph, typename distance, typename label, int R>
void verify_CBFS_batch(parlay::sequence<parlay::sequence<vertex>>& vertices, const graph& G, parlay::sequence<std::array<label,R>>& S, parlay::sequence<distance>& D){
  const distance INF=(1<<sizeof(distance)*8-1)-1;
  size_t num_batches= vertices.size();
  size_t n = G.size();
  parlay::sequence<distance> answer(n);
  for (size_t i = 0; i<num_batches; i++){
    for (size_t j = 0; j<vertices[i].size(); j++){
      if (j!=0 && vertices[i][j]==vertices[i][0]){break;}
      parlay::parallel_for(0, n, [&](size_t i){answer[i]=INF;});
      BFS(vertices[i][j], G, G, answer);
      for (size_t v = 0; v<n; v++){
        distance d_true = answer[v];
        distance d_query=D[v*num_batches+i];
        if (d_true == INF){continue;}
        bool changed = false;
        for (size_t r = 0;r<R; r++){
          if (S[v*num_batches+i][r]&(1ul<<j)){
            d_query -= (R-r-1);
            changed=true;
            break;
          }
        }
        // d_query = (d_query==INF)? INF:(S[v*num_batches+i][0]&(1ul<<j))?d_query-1:
          // (S[v*num_batches+i][1]&(1ul<<j))? d_query: d_query+1;
        if (changed && d_query!=d_true){
          printf("source: %d, batch_id %d, vertex_id %d, target: %d, d_true: %d, d_query: %d, D: %d, S0: %lu, S1: %lu\n",vertices[i][j], i,j,v,d_true, d_query, D[v*num_batches+i],S[v*num_batches+i][0], S[v*num_batches+i][1]);
          return;
        }else if (!changed && (d_query-d_true>=R)){
          printf("u: %d, v: %d, Out of range!\n", vertices[i][j],v);
          if (d_query==INF){printf("d_query is INF\n");}
        }
      }
    }
    printf("Batch %d PASS\n",i);
  }
}