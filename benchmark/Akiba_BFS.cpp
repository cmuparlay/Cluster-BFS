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


using vertex = int;
using nested_seq = parlay::sequence<parlay::sequence<vertex>>;
using graph = nested_seq;
using utils = graph_utils<vertex>;
using label=uint64_t;

void Akiba_BFS(parlay::sequence<parlay::sequence<vertex>>& vertices, const graph& G, parlay::sequence<std::array<uint64_t,2>>& S, parlay::sequence<uint8_t>& D){
  size_t V=G.size();
  size_t E=parlay::reduce(parlay::map(G, parlay::size_of()));
  const uint8_t INF8 = 100;
  std::vector<bool> usd(V, false);  // Used as root? (in new label)
  size_t kNumBitParallelRoots=vertices.size();
  {
    std::vector<uint8_t> tmp_d(V);
    std::vector<std::pair<uint64_t, uint64_t> > tmp_s(V);
    std::vector<int> que(V);
    std::vector<std::pair<int, int> > sibling_es(E);
    std::vector<std::pair<int, int> > child_es(E);

    for (int i_bpspt = 0; i_bpspt < kNumBitParallelRoots; ++i_bpspt) {
      fill(tmp_d.begin(), tmp_d.end(), INF8);
      fill(tmp_s.begin(), tmp_s.end(), std::make_pair(0, 0));

      vertex r = vertices[i_bpspt][0];
      int que_t0 = 0, que_t1 = 0, que_h = 0;
      que[que_h++] = r;
      tmp_d[r] = 0;
      que_t1 = que_h;
      for (auto i = 1; i<vertices[i_bpspt].size(); i++){
        vertex v = vertices[i_bpspt][i];
        if (v==r) break;
        que[que_h++]=v;
        tmp_d[v] = 1;
        tmp_s[v].first = 1ULL << i;
      }

      for (int d = 0; que_t0 < que_h; ++d) {
        int num_sibling_es = 0, num_child_es = 0;

        for (int que_i = que_t0; que_i < que_t1; ++que_i) {
          int v = que[que_i];

          for (size_t i = 0; i < G[v].size(); ++i) {
            int tv = G[v][i];
            int td = d + 1;

            if (d > tmp_d[tv]);
            else if (d == tmp_d[tv]) {
              if (v < tv) {
                sibling_es[num_sibling_es].first  = v;
                sibling_es[num_sibling_es].second = tv;
                ++num_sibling_es;
              }
            } else {
              if (tmp_d[tv] == INF8) {
                que[que_h++] = tv;
                tmp_d[tv] = td;
              }
              child_es[num_child_es].first  = v;
              child_es[num_child_es].second = tv;
              ++num_child_es;
            }
          }
        }

        for (int i = 0; i < num_sibling_es; ++i) {
          int v = sibling_es[i].first, w = sibling_es[i].second;
          tmp_s[v].second |= tmp_s[w].first;
          tmp_s[w].second |= tmp_s[v].first;
        }
        for (int i = 0; i < num_child_es; ++i) {
          int v = child_es[i].first, c = child_es[i].second;
          tmp_s[c].first  |= tmp_s[v].first;
          tmp_s[c].second |= tmp_s[v].second;
        }

        que_t0 = que_t1;
        que_t1 = que_h;
      }

      for (int v = 0; v < V; ++v) {
        D[v*kNumBitParallelRoots+i_bpspt] = tmp_d[v];
        S[v*kNumBitParallelRoots+i_bpspt][0] = tmp_s[v].first;
        S[v*kNumBitParallelRoots+i_bpspt][1] = tmp_s[v].second & ~tmp_s[v].first;
      }
    }
  }
}

void verify_Akiba(parlay::sequence<parlay::sequence<vertex>>& vertices, const graph& G, parlay::sequence<std::array<uint64_t,2>>& S, parlay::sequence<uint8_t>& D){
  const uint8_t INF=127;
  size_t num_batches= vertices.size();
  size_t n = G.size();
  parlay::sequence<uint8_t> answer(n);
  // size_t set_size = k>>3;
  for (size_t i = 0; i<num_batches; i++){
    BFS(vertices[i][0], G, G, answer);
    for (size_t v=0;v<n; v++){
      if (answer[v] == INF){continue;}
      if (answer[v]!=D[v*num_batches+i]){
        printf("batch %d, id: %d, source: %d, target: %d, d_true: %d, d_query: %d\n", i,0,vertices[i][0],v,answer[v], D[v*num_batches+i]);
        return;
      }
    }
    for (size_t j = 1; j<vertices[i].size(); j++){
      if (vertices[i][j]==vertices[i][0]){break;}
      parlay::parallel_for(0, n, [&](size_t i){answer[i]=INF;});
      BFS(vertices[i][j], G, G, answer);
      for (size_t v = 0; v<n; v++){
        uint8_t d_true = answer[v];
        uint8_t d_query=D[v*num_batches+i];
        if (d_true == INF){continue;}
        if (S[v*num_batches+i][0]&(1ul<<j)){d_query-=1;}
        else if (S[v*num_batches+i][1]&(1ul<<j)){d_query-=0;}
        else{
          d_query+=1;
        }
        if (d_true != d_query){
          printf("batch %d, id: %d, source: %d, target: %d, d_true: %d, d_query: %d\n", i,0,vertices[i][0],v,answer[v], D[v*num_batches+i]);
          return;
        }
      }
    }
    printf("Batch %d PASS\n",i);
  }
}


int main(int argc, char* argv[]) {
  auto usage = "Usage: Akiba_BFS <filename> || multi_BFS <n> <k>";
  if (argc < 2) {std::cout << usage << std::endl; return 0;}
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
  CommandLine P(argc, argv);
  int t = P.getOptionInt("-t", 3);
  size_t ns = P.getOptionInt("-ns", 10);
  // size_t k = P.getOptionInt("-k",sizeof(label)*8);
  size_t k = 64;
  size_t set_size = 1;
  parlay::sequence<parlay::sequence<vertex>> seeds(ns);
  for (size_t i = 0; i<ns; i++){seeds[i]=parlay::sequence<vertex>(k);}
  select_seeds<graph, vertex,label>(G, seeds, ns, k);
  bool verify = P.getOption("-v");
  parlay::sequence<std::array<label,2>> S;
  parlay::sequence<uint8_t> D;
  S=parlay::sequence<std::array<label,2>>(n*ns);
  D=parlay::sequence<uint8_t>(n*ns);
  Akiba_BFS(seeds,G, S, D);
  if (verify){
    verify_Akiba(seeds, G, S,D);
  }
  parlay::internal::timer BFS_T("Akiba BFS");
  for (int i = 0; i<t;i++){
    Akiba_BFS(seeds, G, S,D);
    BFS_T.next("");
  }
  printf("average cluster BFS time: %f\n", BFS_T.total_time()/t);
}