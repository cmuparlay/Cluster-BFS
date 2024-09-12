#ifndef APPROX_UTILS_H
#define APPROX_UTILS_H
#include <iostream>
#include <string>
#include <unordered_set>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <iostream>
#include <string>
#include <parlay/io.h>
#include "ligra_light.h"

template<typename vertex, typename distance>
parlay::sequence<std::pair<std::pair<vertex, vertex>,distance>> 
read_ground_truth(const std::string& filename){
  parlay::sequence<std::pair<std::pair<vertex, vertex>,distance>> ground_truth;
  auto str = parlay::file_map(filename);
  auto tokens = parlay::tokens(str, [] (char c) {return c == '\n' || c == ' ';});
  size_t n = parlay::chars_to_long(tokens[0]);
  if (tokens.size() != 3*n+1) {
    std::cout << "Bad file format, read_graph_from_file expects:\n"
              << "<n> <m> <degree 0> <degree 1> ... <degree n-1> <edge 0> ... <edge m-1>\n"
              << "Edges are sorted and each difference encoded with respect to the previous one."
               << "First per vertex is encoded directly." << std::endl;
    return ground_truth;
  }
  ground_truth=parlay::sequence<std::pair<std::pair<vertex, vertex>,distance>>(n);
  parlay::parallel_for(0,n,[&](size_t i){
    vertex u = parlay::chars_to_uint(tokens[3*i+1]);
    vertex v = parlay::chars_to_uint(tokens[3*i+2]);
    distance d = parlay::chars_to_uint(tokens[3*i+3]);
    ground_truth[i]=std::pair(std::pair(u,v),d);
  });
  return ground_truth;
}

template<typename vertex, typename distance>
void write_distribution(const std::string& filename,  
    parlay::sequence<std::pair<distance,distance>>& _pairs){
  auto pairs = parlay::sort(_pairs);
  size_t n = pairs.size();
  distance large_d=pairs[n-1].first;
  distance small_d=pairs[0].first;
  std::ofstream out_file;
  out_file.open(filename);
  out_file << "distance range: "<< (uint32_t)small_d << " "<<(uint32_t)large_d << std::endl;
  auto iota = parlay::internal::delayed_tabulate(n, [](vertex i) { return i; });
  auto starts = parlay::filter(iota, [&] (size_t i) {
	  return (i==0) || pairs[i].first!= pairs[i-1].first;});
  size_t m = starts.size();
  parlay::sequence<parlay::sequence<distance>> bucket(large_d-small_d+1);
  parlay::parallel_for(0, m, [&] (size_t i) {
    size_t start = starts[i];
    size_t end = ((i == m-1) ? n : starts[i+1]);
    bucket[pairs[start].first-small_d] =
	    parlay::map(pairs.cut(start, end), [&] (auto kv) {
	  return std::move(kv.second);}, 1000);});
  for (size_t i = 0; i<bucket.size(); i++){
    distance d_true = small_d+i;
    distance d = d_true;
    size_t cnt = 0;
    if (bucket[i].size()==0){continue;}
    out_file << "true distance: " << (uint32_t)d_true<<std::endl;
    for (size_t j = 0; j<bucket[i].size(); j++){
      if (bucket[i][j]==d){cnt++;}
      else{
        out_file << (uint32_t)d << ": " << cnt << std::endl;
        d = bucket[i][j];
        cnt=1;
      }
    }
    out_file << (uint32_t)d << ": " << cnt << std::endl;
  }
}
template<typename vertex, typename distance>
void write_answer_to_file(const std::string& filename, 
  parlay::sequence<std::pair<std::pair<vertex, vertex>, distance>>& ground_truth,parlay::sequence<std::pair<distance, distance>>& dist){
  std::ofstream out_file;
  out_file.open(filename);
  size_t n = ground_truth.size();
  out_file << n << std::endl;
  for (size_t i = 0; i<n; i++){
    out_file << ground_truth[i].first.first << " "\
       << ground_truth[i].first.second << " "\
       << unsigned(dist[i].second)<< std::endl;
  }
  out_file.close();
}
template<typename vertex>
parlay::sequence<vertex> order_by_degrees(const parlay::sequence<parlay::sequence<vertex>>& G){
  size_t n = G.size();
  auto sizes = parlay::tabulate(n, [&](vertex j){return std::make_pair(G[j].size(), j);});
  auto cmp=[&](auto a ,auto b){
    if (a.first!=b.first){return a.first>b.first;}
    return parlay::hash32(a.second)>parlay::hash32(b.second);
  };
  parlay::sort_inplace(sizes, cmp);
  auto orders = parlay::map(sizes, [&](std::pair<size_t,vertex> p){return p.second;});
  return orders;
}

template<typename vertex>
parlay::sequence<parlay::sequence<vertex>>
reorder_graph(const parlay::sequence<parlay::sequence<vertex>>& G, 
          const parlay::sequence<vertex>& orders){
  size_t n = G.size();
  parlay::sequence<vertex> get_order(n);
  parlay::parallel_for(0, n, [&](size_t i){
    get_order[orders[i]]=i;
  });
  return parlay::map(G,[&](auto ngb){
    return parlay::sort(ngb, [&](vertex a, vertex b){
      return get_order[a]<get_order[b];});
  });
}

template<typename graph, typename vertex, typename label_type>
void select_seeds(graph& G, parlay::sequence<parlay::sequence<vertex>>& seeds, size_t num_batch, size_t set_size){
  size_t n = G.size();
  printf("select seeds from a 1-hop star\n");
  // size_t set_size = sizeof(label_type)*8;
  size_t r = 0;
  auto orders = parlay::random_permutation((vertex)G.size());
  auto vertices = parlay::filter(orders, [&](vertex v){return G[v].size()>=set_size;});
  // auto vertices = parlay::filter(orders, [&](vertex v){return G[v].size()>=2048;});
  printf("%d candidates\n", vertices.size());
  parlay::sequence<vertex> get_order(n);
  parlay::parallel_for(0, n, [&](size_t i){
    get_order[orders[i]]=i;
  });
  for (auto v : vertices){
    // if (G[v].size() < set_size) continue;
    seeds[r][0]=v;
    size_t ns=1;
    auto neighbors = parlay::sort(G[v], [&](vertex a, vertex b){
      return get_order[a]<get_order[b];});
    for (auto u:neighbors){
      if (u==v){continue;}
      seeds[r][ns++]=u;
      if (ns==set_size){break;}
    }
    for (;ns<set_size;ns++){
      seeds[r][ns]=v;
    }
    r++;
    if (r==num_batch){break;}
  }
}

template<typename graph, typename vertex, typename label_type>
void select_seeds(graph& G, parlay::sequence<parlay::sequence<vertex>>& seeds, size_t num_batch){
  size_t set_size = sizeof(label_type)*8;
  size_t r = 0;
  auto orders = parlay::random_permutation((vertex)G.size());
  for (auto v : orders){
    if (G[v].size() < set_size) continue;
    seeds[r][0]=v;
    for (size_t ns = 1; ns<set_size; ns++){
      seeds[r][ns]=G[v][ns-1];
    }
    r++;
    if (r==num_batch){break;}
  }
}

template<typename graph, typename vertex, typename label_type>
void select_seeds2(graph& G, parlay::sequence<parlay::sequence<vertex>>& seeds, size_t num_batch, size_t set_size){
  printf("select seeds from a 2-hop star\n");
  // size_t set_size = sizeof(label_type)*8;
  size_t r = 0;
  auto orders = parlay::random_permutation((vertex)G.size());
  auto vertices = parlay::filter(orders, [&](vertex v){return G[v].size()>=int(log(set_size));});
  for (auto v : vertices){
    size_t ns = 0;
    seeds[r][ns++]=v;
    std::unordered_set<vertex> neighbors;
    neighbors.reserve(set_size);
    for (auto i : G[v]){
      if (i!=v) neighbors.insert(i);
      for (auto j:G[i]){
        if (j!=v)neighbors.insert(j);
      }
    }
    for (auto u:neighbors){
      seeds[r][ns++]=u;
      if (ns==set_size){break;}
    }
    for (;ns<set_size;ns++){
      seeds[r][ns]=v;
    }
    r++;
    if (r==num_batch){break;}
  }
}

template<typename graph, typename vertex, typename label_type>
void select_seeds3(graph& G, parlay::sequence<parlay::sequence<vertex>>& seeds, size_t num_batch, size_t set_size){
  size_t n = G.size();
  printf("select seeds from a 3-hop star\n");
  // size_t set_size = sizeof(label_type)*8;
  size_t r = 0;
  auto orders = parlay::random_permutation((vertex)G.size());
  auto vertices = parlay::filter(orders, [&](vertex v){return G[v].size()>=int(log(set_size));});
  for (auto source : vertices){
    size_t ns = 0;
    seeds[r][ns++]=source;
    auto neighbors = parlay::tabulate<std::atomic<bool>>(G.size(), [&] (long i) {
    return (i==source) ? true : false; });
    auto edge_f = [&] (vertex u, vertex v) -> bool {
      bool expected = false;
      return neighbors[v].compare_exchange_strong(expected, true);};
    auto cond_f = [&] (vertex v) { return !neighbors[v];};
    auto frontier_map = ligra::edge_map(G, G, edge_f, cond_f);

    auto frontier = ligra::vertex_subset<vertex>(source);
    size_t round = 0;
    // nested_seq frontiers;
    while (frontier.size() > 0 && round <3) {
      frontier = frontier_map(frontier);
      round++;
    }
    for (auto u:orders){
      if (!neighbors[u].load()) continue;
      if (u==source) continue;
      seeds[r][ns++]=u;
      if (ns==set_size){break;}
    }
    for (;ns<set_size;ns++){
      seeds[r][ns]=source;
    }
    r++;
    if (r==num_batch){break;}
  }
}

template <typename label, template<typename,typename> class NodeInfo, typename graph, typename vertex,typename distance>
void multiBFS_verifier(parlay::sequence<vertex>& vertices,graph& G,
                    parlay::sequence<NodeInfo<label,distance>>&ans_result){
  #if defined(TWO_LAYER)
  printf("TWO_LAYER verifier\n");
  #endif
  bool check=false;
  parlay::sequence<distance> answer(G.size());
  const distance INF=(1<<(sizeof(distance)*8-1))-1;
  parlay::parallel_for(0, G.size(), [&](size_t i){answer[i]=INF;});
  BFS(vertices[0], G, G, answer);
  parlay::parallel_for(0, G.size(), [&](vertex v){
      distance d = answer[v];
      distance dd = ans_result[v].d.load();
      if (dd != d ){
        printf("center failed at %u, true d: %u, got %u\n", v,d,dd);
        check|=true;
      }
  });
  if (check){printf("check 0 fail\n");return;}
  for (size_t ns = 1; ns<vertices.size(); ns++){
    if (vertices[ns]==vertices[0]) continue;
    parlay::parallel_for(0, G.size(), [&](size_t i){answer[i]=INF;});
    BFS(vertices[ns], G, G, answer);
    size_t label_u = 1ul<< ns;
    parlay::parallel_for(0, G.size(), [&](vertex v){
      distance d = answer[v];
      label s1 = ans_result[v].s1.load();
      label s0 = ans_result[v].s0.load();
      #ifdef TWO_LAYER
      label s2 = ans_result[v].s2.load();
      label s3 = ans_result[v].s3.load();
      #endif
      distance dd = ans_result[v].d.load();
      if (d == dd){
        #if defined(TWO_LAYER)
        bool fail = !(label_u & s2) && (dd!=INF);
        #else
        bool fail = !(label_u & s1) && (dd!=INF);
        #endif
        if (fail){
          #ifdef TWO_LAYER
          printf("d layer fail, u:%u ns:%u v:%u d:%u, dd:%u, s0:%u, s1:%u, s2:%u\n", vertices[ns],ns, v,d, dd,s0,s1,s2);
          #else
          printf("d layer fail, center: %u u:%u ns:%u v:%u d:%u, dd:%u, s0:%u, s1:%u\n", vertices[0], vertices[ns],ns,v,d, dd,s0,s1);
          #endif
          check|=true;
        }
      }else if (dd-1 == d){
        #if defined(TWO_LAYER)
        bool fail = !(label_u&s1);
        #else
        bool fail = !(label_u&s0);
        #endif
        if (fail){
          printf("d-1 layer fail, u:%u ns:%u v:%u d:%u\n", vertices[ns],ns,v,d);
          check|=true;
        }
      }
      #if defined(TWO_LAYER)
      if (dd-2 == d){
        if (!(label_u&s0)){
          printf("d-2 layer fail, u:%u ns:%u v:%u d:%u, s0:%u, s1:%u, s2:%u\n", vertices[ns], ns, v, d, s0,s1,s2);
          check|=true;
        }
      }
      if (dd+1==d){
        if (!(label_u&s3)){
          printf("d+1 layer fail, u: %u ns: %u v: %u d: %u, s3: %u\n", vertices[ns], ns, v, d, s3);
          check|=true;
        }
      }
      #endif
    });
    if (check){printf("check %ld fail\n", ns);return;}
  }
  printf("PASS\n");
}

template <typename vertex, typename graph, typename dist_t>
std::pair<dist_t, vertex> seq_BFS(vertex s, graph& G, parlay::sequence<dist_t>& d){
  parlay::sequence<vertex> Q(G.size());
  size_t head = 0; size_t tail = 0;
  dist_t INF=(1<<sizeof(dist_t)*8-1)-1;
  Q[head++]=s;
  d[s]=0;
  while (tail<head){
    vertex u = Q[tail++];
    dist_t d_u=d[u];
    for (auto v:G[u]){
      if (d[v]==INF){
        d[v]=d_u+1;
        Q[head++]=v;
      }
    }
  }
  return std::pair(d[Q[head]],head);
}


template <typename vertex, typename graph, typename dist_t>
void remove_smallCC(const graph& G, parlay::sequence<vertex>& orders, parlay::sequence<bool>& mark){
  dist_t INF=(1<<sizeof(dist_t)*8-1)-1;
  size_t n = G.size();
  parlay::sequence<dist_t> distance(n, INF);
  for (auto v:orders){
    auto p=seq_BFS(v,G,distance);
    if (p.second>std::ceil(n/2.0)){
      parlay::parallel_for(0,n, [&](size_t i){distance[i]=INF;});
      seq_BFS(v,G,distance);
      parlay::parallel_for(0,n,[&](size_t i){mark[i]=(distance[i]==INF);});
      return;
    }
  } 
}

#endif