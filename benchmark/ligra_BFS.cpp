#include <iostream>
#include <string>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/internal/get_time.h>
#include <parlay/slice.h>
#include <parlay/io.h>

#include "BFS_ligra.h"
#include "ligra_light.h"
#include "graph_utils.h"
#include "parseCommandLine.h"
#include "utils.h"

// **************************************************************
// Driver
// **************************************************************
#ifndef label
  using label = uint64_t;
#endif

using vertex = int;
using nested_seq = parlay::sequence<parlay::sequence<vertex>>;
using graph = nested_seq;
using utils = graph_utils<vertex>;
using dist_t = uint8_t;

template <typename vertex, typename graph, typename dist_t>
void verifier(vertex & s,graph& G, parlay::sequence<dist_t>&ans){
  // bool fail=false;
  dist_t INF=(1<<sizeof(dist_t)*8-1)-1;
  parlay::sequence<dist_t> answer(G.size(), INF);
  BFS(s, G, G, answer);
  for (size_t i = 0; i<G.size(); i++){
    if (answer[i]!=ans[i]){
      printf("Fail at %u true %u answer %u\n", i, answer[i], ans[i]);
      return;
    }
  }
  printf("PASS\n");
}


int main(int argc, char* argv[]) {
  auto usage = "Usage: BFS <filename>";
  if (argc < 2) {std::cout << usage << std::endl; return 0;}
  dist_t INF=(1<<sizeof(dist_t)*8-1)-1;
  CommandLine P(argc, argv);
  long n = 0;
  graph G;
  try { n = std::stol(argv[1]); }
  catch (...) {}
  if (n == 0) {
    G=utils::read_graph_from_bin(argv[1]);
    n = G.size();
  } else {
    int _k = std::atoi(argv[2]);
    G=utils::random_graph(n,_k);
  }
  utils::print_graph_stats(G);
  int t = P.getOptionInt("-t", 5);
  size_t ns = P.getOptionInt("-ns", 10);
  size_t k = P.getOptionInt("-k",sizeof(label)*8);
  parlay::sequence<parlay::sequence<vertex>> seeds(ns);
  for (size_t i = 0; i<ns; i++){seeds[i]=parlay::sequence<vertex>(k);}
  select_seeds<graph, vertex,label>(G, seeds, ns, k);

  parlay::sequence<dist_t> result(G.size());

  printf("Ligra_BFS\n");
  // parlay::parallel_for(0, G.size(), [&](size_t i){result[i]=INF;});
  BFS(seeds[0][0], G,G,result);
  parlay::internal::timer parallelT("Ligra_BFS");
  for (auto seed:seeds){
    for (auto s:seed){
      BFS(s,G,G,result);
    }
    parallelT.next("");
  }
  printf("Total time for %lu Ligra_BFS: %lf\n", ns*k, parallelT.total_time());
  printf("average time for %lu Seq_BFS: %lf\n", k, parallelT.total_time()/ns);
}
