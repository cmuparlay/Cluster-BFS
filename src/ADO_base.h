#include <atomic>
#include <utility>
#include <limits>
#include <unordered_set>
#include <bitset>
#include <xmmintrin.h>
#include <math.h>


#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <absl/container/flat_hash_map.h>
#include "BFS_ligra.h"
#include "utils.h"
#include "cluster_BFS.h"


template<typename graph, typename distance, typename vertex>
class LandmarkLabeling_base {
public:
  const graph& G;
  size_t num_seeds;
  size_t n;
  // const distance INF8=100;  // For unreachable pairs
  const distance INF = (1<<(sizeof(distance)*8-1))-1;
  parlay::sequence<vertex> seeds;
  parlay::sequence<bool> mark;
  parlay::sequence<parlay::sequence<distance>> Index;
 
  void select_seeds(parlay::sequence<vertex>& orders);
  // Constructs an index from a graph. 
  // Return true if construct successfully
  void ConstructIndex();

  // Returns distance vetween vertices |v| and |w| if they are connected.
  // Otherwise, returns |INF8|.
  distance Query(vertex v, vertex w);
  // void Local_BFS(vertex r, std::pair<vertex,distance>* que);
  distance query_BiBFS(vertex v, vertex w, size_t search_size);
  distance Query_local(vertex v, vertex w, size_t search_size);
  LandmarkLabeling_base(graph& _G, size_t k): G(_G), num_seeds(k){
    n=G.size();
    seeds = parlay::sequence<vertex>(num_seeds);
    mark = parlay::sequence<bool>(n);
    Index = parlay::sequence<parlay::sequence<distance>>(n);
    for (size_t i = 0; i<n; i++){
      Index[i]=parlay::sequence<distance>(num_seeds);
    }
  }
};

template<typename graph, typename distance, typename vertex>
void LandmarkLabeling_base<graph, distance, vertex>::
select_seeds(parlay::sequence<vertex>& orders){
  // parlay::parallel_for(0,n,[&](size_t i){mark[i]=false;});
  remove_smallCC<vertex,graph,distance>(G, orders, mark);
  size_t ns=0;
  for (auto v:orders){
    if (mark[v]) continue;
    seeds[ns++]=v;
    mark[v]=true;
    if (ns==num_seeds)break;
  }
}

template<typename graph, typename distance, typename vertex>
void LandmarkLabeling_base<graph, distance, vertex>::
ConstructIndex(){
  parlay::parallel_for(0,n,[&](size_t i){
    for (size_t j = 0; j<seeds.size(); j++){Index[i][j]=INF;}
  });
  parlay::sequence<distance> answer(n);
  // printf("Construct Index, INF:%d\n", INF);
  parlay::internal::timer t("Prefix",false);
  for (size_t i = 0; i<seeds.size(); i++){
    vertex s = seeds[i];
    parlay::parallel_for(0,n,[&](size_t j){answer[j]=INF;});
    BFS(s, G,G, answer);
    parlay::parallel_for(0, n, [&](size_t v){Index[v][i]=answer[v];});
    t.next("construct");
  }

}
template<typename graph, typename distance, typename vertex>
distance LandmarkLabeling_base<graph, distance, vertex>::
Query(vertex u, vertex v){
  size_t num_seeds = seeds.size();
  distance min_dist = INF;
  for (size_t i = 0; i<num_seeds; i++){
    distance d = Index[u][i]+Index[v][i];
    if(d<min_dist) min_dist=d;
  }
  return min_dist;
}

template<typename graph, typename distance, typename vertex>
distance LandmarkLabeling_base<graph, distance, vertex>::
query_BiBFS(vertex u, vertex v, size_t search_size){
  absl::flat_hash_map<std::pair<vertex,distance>, distance> vis; 
  vis.reserve(search_size);
  vertex Qu[search_size];
  vertex Qv[search_size];
  vertex* Q[2]={Qu,Qv};
  vertex head0[2]={0,0};
  vertex head1[2]={0,0};
  vertex tail[2]={0,0};
  Q[0][tail[0]++]=u;
  Q[1][tail[1]++]=v;
  head1[0]=tail[0];
  head1[1]=tail[1];
  vis.insert({std::pair(u,0),0});
  vis.insert({std::pair(v,1),0});
  distance min_dist=(distance)((1<<(sizeof(distance)*8-1))-1);
  vertex n_visit=2;
  while (head1[0]>head0[0] && head1[1]>head0[1]){
    uint8_t u_small=(head1[0]-head0[0])>(head1[1]-head0[1]);
    vertex start = head0[u_small];
    vertex end = head1[u_small];
    distance d = vis.find(std::pair(Q[u_small][start],u_small))->second;
    for (vertex i=start; i<end;i++){
      vertex uu = Q[u_small][i];
      for (auto vv:G[uu]){
        if (!mark[vv]){
          auto find_vv = vis.find(std::pair(vv,u_small));
          if (find_vv==vis.end()){
            vis.insert({std::pair(vv,u_small), d+1});
            n_visit++;
            Q[u_small][tail[u_small]++]=vv;
            auto find_vv_other = vis.find(std::pair(vv, 1-u_small));
            if (find_vv_other!= vis.end()){
              distance new_d = find_vv_other->second+d+1;
              // printf("new_d: %u \n", new_d);
              return new_d;
            }
            if (tail[u_small]==search_size || n_visit== search_size){
              // printf("n_visit: %u tail_u: %u tail_v: %u\n",n_visit, tail[0], tail[1]);
              return min_dist;
            }
          }
        }
      }
    }
    head0[u_small]=head1[u_small];
    head1[u_small]=tail[u_small];
  }
  return min_dist;
}


template<typename graph, typename distance, typename vertex>
distance LandmarkLabeling_base<graph, distance, vertex>::
Query_local(vertex u, vertex v, size_t search_size){
  distance d_index= Query(u,v);
  if (search_size==0){return d_index;}
  distance d_local = query_BiBFS(u,v,search_size);
  return std::min(d_index, d_local);
}

