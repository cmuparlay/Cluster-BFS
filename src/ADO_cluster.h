#include <atomic>
#ifndef LANDMARK_LABELING_BATCH_H
#define LANDMARK_LABELING_BATCH_H
#include <utility>
#include <limits>
#include <unordered_set>
#include <bitset>
#include <xmmintrin.h>
#include <math.h>
#include <queue>


#include <parlay/primitives.h>
#include <parlay/sequence.h>
// #include "multi_BFS.h"
// #include "batch_BFS.h"
#include <absl/container/flat_hash_map.h>
#include "utils.h"
#include "cluster_BFS_batch.h"


template<typename graph, typename distance, typename vertex, typename label, int R>
class LandmarkLabeling_batch {
public:
  const graph& G;
  size_t num_batch;
  size_t set_size;
  size_t n;
  // const distance INF8=100;  // For unreachable pairs
  const distance INF=(1<<(sizeof(distance)*8-1))-1;
  parlay::sequence<parlay::sequence<vertex>> seeds;
  parlay::sequence<bool> mark;
  parlay::sequence<std::array<label,R>> S;
  parlay::sequence<distance> D;
  size_t select_seeds(parlay::sequence<vertex>& orders);
  size_t select_seeds2(parlay::sequence<vertex>& orders);
  // Constructs an index from a graph. 
  // Return true if construct successfully
  void ConstructIndex();

  // Returns distance vetween vertices |v| and |w| if they are connected.
  // Otherwise, returns |INF8|.
  distance query_helper(vertex v, vertex w, size_t i);
  distance Query(vertex v, vertex w);
  // void Local_BFS(vertex r, std::pair<vertex,distance>* que);
  distance query_BiBFS(vertex v, vertex w, size_t search_size);
  distance Query_local(vertex v, vertex w, size_t search_size);
  LandmarkLabeling_batch(const graph& _G, size_t k): G(_G), num_batch(k){
    set_size = sizeof(label)*8;
    n = G.size();
    seeds = parlay::sequence<parlay::sequence<vertex>>(num_batch);
    for (size_t i = 0; i<num_batch; i++){seeds[i]=parlay::sequence<vertex>(set_size);}
    mark = parlay::sequence<bool>(n);
    S=parlay::sequence<std::array<label,R>>(num_batch*n);
    D=parlay::sequence<distance>(num_batch*n);
  }
};

template<typename graph, typename distance, typename vertex, typename label, int R>
size_t LandmarkLabeling_batch<graph, distance, vertex, label, R>::
select_seeds(parlay::sequence<vertex>& orders){
  // parlay::parallel_for(0, n, [&](size_t i){mark[i]=false;});
  remove_smallCC<vertex,graph,distance>(G, orders, mark);
  parlay::sequence<vertex> get_order(n);
  parlay::parallel_for(0, n, [&](size_t i){
    get_order[orders[i]]=i;
  });
  parlay::sequence<vertex> sources;
  size_t r = 0;
  size_t actual_size=0;
  for (vertex i=0;i<n;i++){
    vertex v = orders[i];
    if (!mark[v]){
      mark[v]=true;
      size_t ns=0;
      seeds[r][ns++]=v;
      sources.push_back(get_order[v]);
      for (auto u:G[v]){
        if (!mark[u]){
          mark[u]=true;
          seeds[r][ns++]=u;
          sources.push_back(get_order[u]);
        }
        if (ns == set_size) break;
      }
      actual_size+=ns;
      while(ns<set_size){seeds[r][ns++]=v;}
      r++;
      if (r==num_batch){break;}
    }
  }
  for (;r<num_batch; r++){
    for (size_t ns=0; ns<set_size; ns++){seeds[r][ns]=n;}
  }
  return actual_size;
}

// within a batch, select most importance set_size 2-layer neighbors
template<typename graph, typename distance, typename vertex, typename label, int R>
size_t LandmarkLabeling_batch<graph, distance, vertex, label, R>::
select_seeds2(parlay::sequence<vertex>& orders){
  printf("two layer seeds selection\n");
  // parlay::parallel_for(0, n, [&](size_t i){mark[i]=false;});
  remove_smallCC<vertex,graph,distance>(G, orders, mark);
  parlay::sequence<vertex> get_order(n);
  parlay::parallel_for(0, n, [&](size_t i){
    get_order[orders[i]]=i;
  });
  size_t r = 0;
  size_t actual_size=0;
  parlay::sequence<vertex> sources;
  for (vertex i=0;i<n;i++){
    vertex v = orders[i];
    if (mark[v]) continue;
    mark[v]=true;
    size_t ns=0;
    seeds[r][ns++]=v;
    sources.push_back(get_order[v]);
    size_t header_size=std::min(G[v].size()+1, set_size);
    parlay::sequence<std::pair<vertex, vertex>> headers(header_size);
    for (vertex i = 0; i<header_size; i++){headers[i]=std::pair(i,0);}
    auto cmp = [&](auto left, auto right) {
      vertex u_i = (left.first==0)? v: G[v][left.first-1];
      vertex u=G[u_i][left.second];
      vertex w_i = (right.first==0)? v: G[v][right.first-1];
      vertex w = G[w_i][right.second];
      return get_order[u]<get_order[w];
    };
    std::priority_queue Q(headers.begin(), headers.end(), cmp);
    while (!Q.empty()){
      auto p= Q.top();Q.pop();
      vertex u_i = (p.first==0)? v: G[v][p.first-1];
      vertex u = G[u_i][p.second];
      if (!mark[u]){
        mark[u]=true;
        seeds[r][ns++]=u;
        sources.push_back(get_order[u]);
      }
      if (ns == set_size) break;
      if (p.second+1>=G[u_i].size()) continue;
      Q.push(std::pair(p.first,p.second+1));
    }
    actual_size+=ns;
    // printf("batch %d: ",r);
    // for (int i = 0; i<ns;i++){
    //   printf("%d ", get_order[seeds[r][i]]);
    // }
    // printf("\n");
    while(ns<set_size){seeds[r][ns++]=v;}
    r++;
    if (r==num_batch){break;}
  }
  for (;r<num_batch; r++){
    for (size_t ns=0; ns<set_size; ns++){seeds[r][ns]=n;}
  }
  parlay::sort_inplace(sources);
  return actual_size;
}

template<typename graph, typename distance, typename vertex, typename label, int R>
void LandmarkLabeling_batch<graph, distance, vertex, label,R>::
ConstructIndex(){
  // default set_size is 1
  parlay::internal::timer t("Prefix",false);
  parlay::parallel_for(0, n*num_batch, [&](size_t i){
    for (int j = 0; j<R; j++){
      S[i][j]=0;
    }
    D[i]=INF;
  });
  cluster_BFS_batch<vertex, graph, distance, label, R>(seeds,G, S, D);
  // verify_kBFS_batch<vertex, graph, distance, label, R>(seeds,G,S,D);
  t.next("construct");
}

template<typename graph, typename distance, typename vertex, typename label,int R>
distance LandmarkLabeling_batch<graph, distance, vertex, label,R>::
query_helper(vertex u, vertex v, size_t i){
  size_t index_u = u*num_batch+i;
  size_t index_v = v*num_batch+i;
  distance tmp_d = D[index_u]+D[index_v];
  distance d;
  // R==2
  d =(S[index_u][0]&S[index_v][0])? tmp_d-2*R+2:
        (S[index_u][0]&S[index_v][1])||(S[index_u][1]&S[index_v][0])?
        tmp_d-2*R+3:tmp_d;
  if constexpr (R >2){
    d= (d<tmp_d)?d:
        (S[index_u][0]&S[index_v][2]||S[index_u][1]&S[index_v][1]||S[index_u][2]&S[index_v][0])?tmp_d-2*R+4:
        (S[index_u][1]&S[index_v][2])||(S[index_u][2]&S[index_v][u])?tmp_d-2*R+5:tmp_d;
  }
  // only for R=4
  if constexpr (R>3){
    d=(d<tmp_d)? d: S[index_u][0]&S[index_v][3]||S[index_u][3]&S[index_v][0]? tmp_d-2*R+5: S[index_u][1]&S[index_v][3]||S[index_u][2]&S[index_v][2]||S[index_u][3]&S[index_v][1]? tmp_d-2*R+6: 
    S[index_u][2]&S[index_v][3]||S[index_u][3]&S[index_v][2]? 
    tmp_d-2*R+7: tmp_d;
  }
  return d;
}

template<typename graph, typename distance, typename vertex, typename label, int R>
distance LandmarkLabeling_batch<graph, distance, vertex, label,R>::
Query(vertex u, vertex v){
  distance min_dist = INF;
  for (size_t i = 0; i<num_batch; i++){
    distance d = query_helper(u,v,i);
    if (d < min_dist){min_dist=d;}
  }
  return min_dist;
}


template<typename graph, typename distance, typename vertex, typename label, int R>
distance LandmarkLabeling_batch<graph, distance, vertex, label,R>::
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
  distance min_dist=(distance)((1<<sizeof(distance)*8-1)-1);
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


template<typename graph, typename distance, typename vertex, typename label, int R>
distance LandmarkLabeling_batch<graph, distance, vertex, label,R>::
Query_local(vertex u, vertex v, size_t search_size){
  distance d_index= Query(u,v);
  if (search_size==0){return d_index;}
  distance d_local = query_BiBFS(u,v,search_size);
  return std::min(d_index, d_local);
}

#endif // LANDMARK_LABELING_BATCH_H