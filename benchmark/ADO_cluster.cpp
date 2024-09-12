#include <iostream>
#include <string>

#include <parlay/primitives.h>
#include <parlay/sequence.h>

#include "ADO_cluster.h"
#include "BFS_ligra.h"
#include "graph_utils.h"
#include "parseCommandLine.h"
#include "utils.h"

#ifndef label
  using label = uint64_t;
#endif
using vertex = uint32_t;
using nested_seq = parlay::sequence<parlay::sequence<vertex>>;
using graph = nested_seq;
using utils = graph_utils<vertex>;
#ifndef dist_t
  using dist_t = uint8_t;
#endif
#ifndef Radius
#define Radius 2
#endif

int main(int argc, char **argv) {
  if (argc < 2) {
      std::cerr << "usage: Approx_base GRAPH <-k size_limits> <-v query_file> <-o output_file> <-radius 1/2>" << std::endl;
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
  utils::print_graph_stats(G);
  size_t size_limit = P.getOptionInt("-k", 1024);
  size_t label_size=(sizeof(dist_t)+sizeof(label)*Radius);
  size_t k =std::floor(size_limit/(label_size+0.0));

  auto orders = order_by_degrees<vertex>(G);
  auto newG = reorder_graph(G, orders);

  LandmarkLabeling_batch<graph,dist_t, vertex, label,Radius> Index(newG,k);
  int r = (Radius+1)/2;
  size_t actual_size;
  if (r==1){
    actual_size=Index.select_seeds(orders);
  }else{
    actual_size=Index.select_seeds2(orders);
  }
  printf("size limit is %u Bytes, radius is %d, select %ld batches, actual %ld seeds\n", size_limit,(int)Radius, k, actual_size);
  printf("set size: %ld\n", sizeof(label)*8);
  // std::cout << "seeds: "<<std::endl;
  // for (auto v: Index.seeds[0]){
  //   std::cout << v<< " ";
  // }
  // std::cout << std::endl;
  parlay::internal::timer t1("Approximate Batch");
  int t = P.getOptionInt("-t", 3);
  Index.ConstructIndex();
  t1.next("Construct");
  t1.reset();t1.start();
  for (int i = 0; i<t; i++){
    Index.ConstructIndex();
    t1.next("Construct");
  }
  // vertex u = 35492562, v= 49548026;
  // dist_t d_true = 9;
  // // dist_t d_query = 2;
  // Index.Query(u,v);

  printf("Average time for construct index: %lf\n", t1.total_time()/t);
  size_t search_size = P.getOptionInt("-local", 0);
  // size_t search_sizes[6]={0, 128,256,512,1024,2048};
  if (P.getOption("-v")){
    auto ground_truth = read_ground_truth<vertex,uint32_t>(P.getOptionValue("-v"));
    size_t q = ground_truth.size();
    const dist_t INF=(1<<(sizeof(dist_t)*8-1))-1;
    parlay::sequence<float> distortion(q);
    parlay::sequence<dist_t> dist_q(q);
    std::atomic<uint32_t> cnt;
    t1.reset(); t1.start();
    parlay::parallel_for(0,q,[&](size_t i){
      vertex u = ground_truth[i].first.first;
      vertex v = ground_truth[i].first.second;
      // dist_t d = ground_truth[i].second;
      dist_q[i] = Index.Query_local(u,v,search_size);
      // if (d != d_q) printf("u: %d, v: %d, d_true: %d, d_query: %d\n",u,v,d,d_q);
      // dist_t d_q = std::min(Index.query_helper(u,v,j), prefix_dist[i]);
      // prefix_dist[i]=d_q;
    });
    t1.stop();
    parlay::parallel_for(0, q, [&](size_t i){
      dist_t d = ground_truth[i].second;
      dist_t d_q= dist_q[i];
      if (d_q==INF){parlay::write_add(&cnt, (uint32_t)1);d=INF;}
      distortion[i] = (double)(d_q-d)/(double)(d);
    });
    auto distortion_sum=parlay::reduce(distortion);
    float average_error=((float)distortion_sum)/(q-cnt.load());
    printf("distortion: %f\n", average_error);
    printf("query time for %lu queries: %lf\n", q, t1.total_time());
  }
  // }
  return 0;
}
