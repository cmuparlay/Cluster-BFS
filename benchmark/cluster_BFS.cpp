#include <iostream>
#include <string>

#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/internal/get_time.h>
#include <parlay/slice.h>

#include "cluster_BFS.h"
// #include "BFS_ligra.h"
#include "graph_utils.h"
#include "parseCommandLine.h"
#include "utils.h"

#ifndef label
  using label = uint64_t;
#endif

// **************************************************************
// Driver
// **************************************************************
using vertex = int;
using nested_seq = parlay::sequence<parlay::sequence<vertex>>;
using graph = nested_seq;
using utils = graph_utils<vertex>;
#ifndef dist_t
  using dist_t = uint8_t;
#endif
#ifndef Radius
#define Radius 2
#endif

void single_batch_test(parlay::sequence<parlay::sequence<vertex>>& seeds, parlay::sequence<parlay::sequence<vertex>>& G, int t, bool verify){
  size_t ns = seeds.size();
  size_t k = seeds[0].size();
  size_t n = G.size();
  printf("Radius is %u\n", Radius);
  printf("label type is: uint%lu_t\n", sizeof (label)*8);
  printf("distance type is: uint%lu_t\n",sizeof(dist_t)*8);
  printf("number of batches: %d\n",ns);
  size_t num_batch = ns;
  parlay::sequence<std::array<label,Radius>> S;
  parlay::sequence<dist_t> D;
  S=parlay::sequence<std::array<label,Radius>>(n);
  D=parlay::sequence<dist_t>(n);

  const dist_t INF=(1<<(sizeof(dist_t)*8-1))-1;
  cluster_BFS<vertex, graph, dist_t, label, Radius>(seeds[0],G, S, D);
  if (verify){
    verify_CBFS<vertex, graph, dist_t, label, Radius>(seeds[0],G,S,D);
  }
  
  parlay::internal::timer batchT("cluster BFS");
  for (int i=0; i < t; i++) {
    for (size_t j = 0; j<num_batch; j++){
      // k_BFS_batch<vertex, graph, dist_t, label, Radius>(slice_sequence,G, S, D,set_size);
      cluster_BFS<vertex, graph, dist_t, label, Radius>(seeds[j],G, S, D);
      if(verify){
        verify_CBFS<vertex, graph, dist_t, label, Radius>(seeds[j],G,S,D);
      }
    }
    batchT.next("");
  }
  printf("average cluster BFS time: %f\n", batchT.total_time()/t);
}


int main(int argc, char* argv[]) {
  auto usage = "Usage: batch_BFS <filename> || multi_BFS <n> <k>";
  if (argc < 2) {std::cout << usage << std::endl; return 0;}
  long n = 0;
  graph G;
  G=utils::read_graph_from_bin(argv[1]);
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
  CommandLine P(argc, argv);
  int t = P.getOptionInt("-t", 3);
  size_t ns = P.getOptionInt("-ns", 10);
  size_t k = P.getOptionInt("-k",sizeof(label)*8);
  parlay::sequence<parlay::sequence<vertex>> seeds(ns);
  for (size_t i = 0; i<ns; i++){seeds[i]=parlay::sequence<vertex>(k);}
  int radius = (Radius+1)/2;
  if (radius==1){
    select_seeds<graph, vertex,label>(G, seeds, ns, k);
  }else if (radius==2){
    select_seeds2<graph, vertex,label>(G, seeds, ns, k);
  }else{
    select_seeds3<graph, vertex,label>(G, seeds, ns, k);
  }

  bool verify = P.getOption("-v");
  single_batch_test(seeds, G, t,verify);
}
