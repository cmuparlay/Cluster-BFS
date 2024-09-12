#include <iostream>
#include <string>

#include <parlay/primitives.h>
#include <parlay/sequence.h>

#include "ADO_base.h"
#include "graph_utils.h"
#include "parseCommandLine.h"
#include "utils.h"


using vertex = uint32_t;
using nested_seq = parlay::sequence<parlay::sequence<vertex>>;
using graph = nested_seq;
// using dist_t = uint8_t;
using utils = graph_utils<vertex>;
#ifndef dist_t
  using dist_t = uint8_t;
#endif


int main(int argc, char **argv) {
  if (argc < 2) {
      std::cerr << "usage: Approx_base GRAPH <-k size_limits> <-v query_file>" << std::endl;
      exit(EXIT_FAILURE);
  }
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
  // Connectivity(G);
  
  std::cout << argv[1] << std::endl;
  utils::print_graph_stats(G);
  // size_t k = std::atoi(argv[2]);
  size_t size_limit=P.getOptionInt("-k", 1024);
  size_t k = std::floor(size_limit/(sizeof(dist_t)+0.0));
  auto orders = order_by_degrees<vertex>(G);
  auto newG = reorder_graph(G, orders);
  const dist_t INF=(1<<(sizeof(dist_t)*8-1))-1;

  LandmarkLabeling_base<graph,dist_t, vertex> Index(newG,k);
  Index.select_seeds(orders);
  printf("select %lu seeds\n",k);
  parlay::internal::timer t1("Approximate Base");
  int t = P.getOptionInt("-t", 3);
  Index.ConstructIndex();
  t1.next("Construct");
  t1.reset();t1.start();
  for (int i = 0; i<t; i++){
    Index.ConstructIndex();
    t1.next("Construct");
  }
  printf("Average time for construct index: %lf\n", t1.total_time()/t);
  if (P.getOption("-v")){
    printf("read ground truth\n");
    auto ground_truth = read_ground_truth<vertex, uint32_t>(P.getOptionValue("-v"));
    size_t q = ground_truth.size();
    parlay::sequence<double> distortion(q);
    parlay::sequence<dist_t> dist_q(q); 
    size_t search_size = 0;
    t1.reset();t1.start();
    std::atomic<size_t> cnt;
    parlay::parallel_for(0,q,[&](size_t i){
      vertex u = ground_truth[i].first.first;
      vertex v = ground_truth[i].first.second;
      // dist_t d = ground_truth[i].second;
      dist_q[i] = Index.Query_local(u,v,search_size);
    });
    t1.stop();
    parlay::parallel_for(0,q,[&](size_t i){
      dist_t d = ground_truth[i].second;
      dist_t d_q=dist_q[i];
      if (d_q==INF){parlay::write_add(&cnt, (uint32_t)1);d=INF;}
      distortion[i]=(double)(d_q-d)/(double)(d);
    });
    auto distortion_sum = parlay::reduce(distortion);
    float average_error=((float)distortion_sum)/(q-cnt.load());
    printf("distortion: %f\n", average_error);
    printf("query time for %lu queries: %lf\n", q, t1.total_time());
  }    
}
